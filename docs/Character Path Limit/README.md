# Win32 Application 260 Character Path Limit

The NWNX4 controller handles parameter lists larger than 260 characters; however, Windows by default doesn't allow 
paths exceeding 260 characters. Since Windows 10, you can enable longer paths with a small change to the Windows 
registry.

## Notes

- Accepts a short maximum. In short (pun INTENDED), the NWNX4 controller supports up to a maximum of 32,767 characters.

## Requirements

- Windows 10
- Access to Windows registry preferably via an administrator account

## Instructions

The instructions are split between expanding or collapsing the character path limit.

### Enable long path limit (>260 characters)

1. As an administrator, run the file entitled **Remove 260 Character Path Limit.reg** to expand the limit beyond 260 characters
2. Restart your computer
3. Verify the registry has been updated by using a larger parameter list with the NWNX4 controller exceeding 260 characters

### Disable long path limit (<=260 characters)

1. As an administrator, run the file entitled **Restore 260 Character Path Limit (Default).reg** to collapse the limit below 260 characters
2. Restart your computer
3. Verify the registry has been updated by using a small parameter list with the NWNX4 controller below 260 characters

## Conclusion

In conclusion, you must enable long paths on you Windows host through the registry in order to use long paths, including the
parameter list passed to the NWNX4 controller.
