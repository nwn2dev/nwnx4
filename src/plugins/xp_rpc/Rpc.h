#ifndef NWNX4_RPC_H
#define NWNX4_RPC_H

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "../plugin.h"

class Rpc: public Plugin {
public:
    Rpc();
    ~Rpc();

    bool Init(char* nwnxhome);

    int GetInt(char* sFunction, char* sParam1, int nParam2) { return 0; }
    void SetInt(char* sFunction, char* sParam1, int nParam2, int nValue) {};
    float GetFloat(char* sFunction, char* sParam1, int nParam2) { return 0.0; }
    void SetFloat(char* sFunction, char* sParam1, int nParam2, float fValue) {};
    void SetString(char* sFunction, char* sParam1, int nParam2, char* sValue) {};
    char* GetString(char* sFunction, char* sParam1, int nParam2) { return ""; };

    void GetFunctionClass(char* fClass);
};

#endif //NWNX4_RPC_H
