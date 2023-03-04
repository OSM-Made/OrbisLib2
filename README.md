# OrbisLib2
API Interface for remote control of a jailbroke PS4 console.

# Build Instructions

## PS4
1. Initialize all submodules in ``External`` and build the respective projects.
2. Copy the **'libGoldHEN.sprx'** from its submodule project to ``build/pkg`` and ``build/pkg/Daemons/ORBS30000``.
3. Copy the **'libKernelInterface.sprx'** from its submodule project to ``build/pkg/Daemons/ORBS30000``.
4. Build all of the projects in the Playstation folder.
5. Build the pkg using your choice of tool.

## Windows
1. Initialize the SimpleUI submodule and build it.
1. Restore all nuget packages.
2. To build you must have the .NET 6.0 SDK installed.
3. Now build the orbislib2.dll.
