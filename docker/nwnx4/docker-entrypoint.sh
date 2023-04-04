#!/usr/bin/env bash

set -euxo pipefail

# If the NWN2 stage file does not exist in NWN2 install directory, drop
for file in $(ls /opt/nwn2-stage); do
  if [[ ! -e "/srv/nwn2/$file" ]]; then
    rm -f "/opt/nwn2-stage/$file"
  fi
done

# If the NWN2 install file does not exist in NWN2 stage directory, add
for file in $(ls /srv/nwn2); do
  if [[ ! -e "/opt/nwn2-stage/$file" ]]; then
    ln -s "/srv/nwn2/$file" /opt/nwn2-stage/ && chown nwnx4:nwnx4 -h "/opt/nwn2-stage/$file"
  fi
done

# Clear all files in plugin folder; do this every startup
rm -Rf /etc/nwnx4/plugins/*;

# Copy a plugin if it exists; prefer /srv/nwnx4-user plugins
for file in $(ls /srv/nwnx4-user/plugins/*.dll | xargs -n 1 basename); do
  cp "/opt/nwnx4/plugins/$file" /etc/nwnx4/plugins/ && chown nwnx4:nwnx4 "/etc/nwnx4/plugins/$file"
done

# Copy a plugin if it exists; only add /opt/nwnx4 plugins if it doesn't exist in /srv/nwnx4-user
for file in $(ls /opt/nwnx4/plugins/*.dll | xargs -n 1 basename); do
  if [[ ! -e "/etc/nwnx4/plugins/$file" ]]; then
    cp "/opt/nwnx4/plugins/$file" /etc/nwnx4/plugins/ && chown nwnx4:nwnx4 "/etc/nwnx4/plugins/$file"
  fi
done

# Wine doesn't support NCrypt well; building it here through openssl
CERTIFICATE_PATH="/srv/nwnx4-user/NWNCertificate"
HOSTNAME="CN=Neverwinter Nights"
ALGORITHM="sha384"

if [ ! -e "${CERTIFICATE_PATH}.pfx" ]; then
  # Generate private key
  openssl ecparam -name secp384r1 -genkey -noout -out "${CERTIFICATE_PATH}.key"

  # Generate certificate request
  openssl req -new -key "${CERTIFICATE_PATH}.key" -subj "/${HOSTNAME}" -out "${CERTIFICATE_PATH}.csr"

  # Self-sign certificate
  openssl x509 -req -days 365000 -in "${CERTIFICATE_PATH}.csr" -signkey "${CERTIFICATE_PATH}.key" -sha384 -out "${CERTIFICATE_PATH}.crt" -extfile <(
    echo "[v3_ca]
  basicConstraints = CA:TRUE
  subjectAltName = DNS:${HOSTNAME}"
  )
  openssl x509 -in "${CERTIFICATE_PATH}.crt" -outform DER -out "${CERTIFICATE_PATH}.cer"

  # Combine key and certificate into PKCS12 format
  openssl pkcs12 -export -in "${CERTIFICATE_PATH}.crt" -inkey "${CERTIFICATE_PATH}.key" -out "${CERTIFICATE_PATH}.pfx" -passout pass:

  # Remove certificate request (.csr) and certificate (.crt)
  rm "${CERTIFICATE_PATH}.key"
  rm "${CERTIFICATE_PATH}.csr"
  rm "${CERTIFICATE_PATH}.crt"
fi

# All files in the /srv/nwnx4-user folder must be owned by the nwnx4 user
chown -R nwnx4:nwnx4 /srv/nwnx4-user

# Setup Xvfb; execute command
gosu nwnx4 bash <<-EOF
  Xvfb $DISPLAY -screen 0 1024x768x16 &
  wine reg import /opt/nwn2.reg
EOF

exec gosu nwnx4 "$@"
