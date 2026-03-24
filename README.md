# Mesa3D IRIX port with glide2x backend, no X11

Mesa3D glide port for IRIX.


## Building

### Prerequisites

Install the complete IRIX development environment:

- MIPSPro 7.4.4 compiler (install 7.4, then patch to 7.4.4m)
- Development Foundation 1.3
- Development Libraries February 2002 (latest version)

Build and install the IRIX 3dfx kernel driver:
- https://github.com/sdz-mods/tdfx_irix

Build and install the glide2x IRIX port:
- https://github.com/sdz-mods/glide_irix


### Build and Install

```csh
#clone or copy this repo onto the target system, e.g. /usr/3dfx_irix/MesaFX_irix
cd /3dfx_irix/MesaFX_irix/src
smake -f Makefile.irixfx GLIDE_ROOT=/usr/3dfx_irix/glide_irix

#output products are static libraries
ls /usr/3dfx_irix/MesaFX_irix/lib32/
libGL.a     libGLU.a    libglut.a
```


## Tested with

- IP32, RM7000C CPU, Irix 6.5.30, Voodoo1, tdfx_irix, glide_irix


## License

Source code is licensed under the MIT License.
