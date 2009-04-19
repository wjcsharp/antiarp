
#pragma once

enum NetClass
{
    NC_NetAdapter=0,
    NC_NetProtocol,
    NC_NetService,
    NC_NetClient,
    NC_Unknown
};

HRESULT FindIfComponentInstalled(IN PCWSTR szComponentId);

HRESULT HrInstallNetComponent(IN PCWSTR szComponentId,
                              IN enum NetClass nc,
                              IN PCWSTR szSrcDir);

HRESULT HrUninstallNetComponent(IN PCWSTR szComponentId);


HRESULT HrShowNetAdapters();
HRESULT HrShowNetComponents();
HRESULT HrShowBindingPathsOfComponent(IN PCWSTR szComponentId);

