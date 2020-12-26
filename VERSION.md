# To-Do and Version Plan

## Legacy Versioning Scheme - `DreamBBS X.Y.Z`; abbreviated as `X.Y.Z`
The versioning scheme originated from MapleBBS.

For MapleBBS 3, `Y` is a two digit number, e.g., `3.00` but not `3.0` and `3.02` but not `3.2`.

Before the introduction of the new versioning scheme, the latest version name was `WindTop 3.10 Rev.Beta 3` or `DreamBBS 3.10 Rev.Beta 3`, and the `.Z` part is not actually used.

## Current Versioning Scheme - `DreamBBS vX.Y.Z`; abbreviated as `vX.Y.Z` 
Note the presence of `v`.

Each release using the new versioning scheme has a corresponding legacy version number as well.

For DreamBBS v3, `Y` is an one digit number, e.g., `v3.0` but not `v3.00` and `v3.2` but not `v3.02`. The next major release after DreamBBS v3.3 will be DreamBBS v4.0, so that `Y` will not be larger than `9`.

It is acceptable to omit the `v` prefix for DreamBBS because using `X.Y.Z` along actually does not cause ambiguity:
- DreamBBS derived from MapleBBS not before MapleBBS 2.36, so before DreamBBS v3 (i.e., `X` < `3`), `DreamBBS X.Y` refers to `DreamBBS vX.Y` but not `MapleBBS X.Y`
- DreamBBS v3 and MapleBBS 3 use different numbers of digits for `Y`, so `DreamBBS 3.xx` refers to `MapleBBS 3.xx` but not `DreamBBS v3.xx`, while `DreamBBS 3.x` refers to `DreamBBS v3.x` but not `MapleBBS 3.x`

The versioning scheme is deprecated. The version number `X.Y.Z` will be consistent with the legacy versioning scheme for `DreamBBS 4.0.0`/`DreamBBS v4.0.0` and on.

## Branches of Development
The new development workflow has led to the two main branches seen today: `master` and `develop`.

## `master` Branch (Release Branch; formerly `testing`)
This branch emphasize more on the stability, the easiness to install, and the ability to deploy on different platform and containers, than the `develop` branch.

Versioning Scheme: `vX.Y.Z` or just `X.Y.Z` after `4.0.0`
  - `X`: Major version; indicates significant changes
  - `Y`: Minor version; indicates introduction of new features
  - `Z`: Patch version; indicates bug fixes or security improvement; based on the previous version
  - `v0.Y` refers to the branch for `v0.Y.Z` versions
  - For `X`>0, `vX.0` refers to the branch for `vX.Y.Z` versions

### `v0.95`: Aka. `3.10 REV-BETA4` (`3.10.95`) ( **End-of-Life** )
The new versioning scheme is introduced in this minor version.
- `v0.95.1`: Aka. `3.10 REV-BETA4.1`
- `v0.95.2`: Aka. `3.10 REV-BETA4.2`
- `v0.95.3`: Aka. `3.10 REV-BETA4.3`
- `v0.95.4`: Aka. `3.10 REV-BETA4.4`; planned to be the last release of `v0.95`

It has become end-of-life when `v0.97.0` was released.

### `v0.96` (`3.10.96`) ( **End-of-Life** )
The `REV-BETA` part in the version name is no longer used.
- `v0.96.1`
- `v0.96.2`

Features:
- Sorted common library and refined structure.
- Sample files are moved to another repository.
- Other detailed changes from `current` version.

It has become end-of-life when `v0.97.1` was released.

### `v0.97`: `stratosphere` (`3.10.97`) ( **End-of-Life** )
- `v0.97-RCx`-`v0.97.0`: Pre-release
- `v0.97.1`
- `v0.97.2`
- `v0.97.3`

Features:
  - Simple code test utilities.
  - Refined layout: Trailing whitespaces are removed.

### `v0.98`-`v0.99` (`3.10.98`-`3.10.99`)
The plans have been dropped due to significant changes which require comprehensive tests.

Pre-releases for `v1.0` were planned instead:
  - [x] `v1.0-alpha1`
  - [x] `v1.0-alpha2`
  - [x] `v1.0-alpha3`
  - [x] `v1.0-beta1`
  - [x] `v1.0-RC1`
  - [x] `v1.0-RC2`
  - [x] `v1.0-RC3`

### `v1.X`: `Tensure` series (`3.11.X`)
- `v1.0.0`: `rimuru`
- `v1.1.0`: `milim`; backports and improvement from `current`
  - Features:
    - [x] Screen resizing feature.
- `v1.1.1`
  - Features:
    - [x] `pfterm`, which was ported in this release version.
    - Others: Please see <https://github.com/ccns/dreambbs/releases/tag/v1.1.0>.
- `v1.2.0`: `shizu`
- `v1.3.0`: `veldora`

### `v2.X`: `Fate` series (`3.12.X`) ( **Latest Release Branch** )
- `v2.0.0`: `artoria`
  - Features in plan:
    - [x] Option for disabling DSO.
    - [ ] BBS-Lua/BBS-Ruby.
    - [x] SHA-256-encrypted password support.
    - [x] Preliminary WebSocket support, based on relevant patches to Maple3 and PttBBS.
- `v2.1.0`: `gilgamesh`
    - Features:
      - Support building with CMake

### `2020 vX` (`v3.X`); `Four Symbol` series (`3.20.X`) ( Upcoming Release Branch )
The full name of this major version is `DreamBBS-2020 vX`.

This will be the last major version whose version number is different from the legacy version number.

Due to the shortage of development resources, from this major version and on, only the latest release branch is officially maintained.

- `2020 v0` (`v3.0`)
- `2020 v1` (`v3.1`)
- `2020 v2` (`v3.2`)
- `2020 v3` (`v3.3`); planned to be the last release of `v3.X`

Features in plan:
  - [x] Hot-swapping of DSO.
  - [ ] Built-in Big5-UAO/UTF-8 conversion support.
  - [ ] Data races prevention using atomic variables and `pthread` mutexes.
  - [ ] Replace System V shared memory API calls with POSIX shared memory API.
  - [ ] Support forcing use 32-bit int type variables in 64-bit environment when BBS is compiled natively.
  - [ ] Support using 64-bit `time_t` to make the BBS system to be immune to the 2038 problem.
    - **48** different structs need to be transformed
      - Not feasible in short-term
  - [x] Full IPv6 support.
  - [ ] Refactor out boilerplate code in xover list system
  - [ ] Merge popup menu systems into main menu system
  - [ ] Merge main menu system into xover list system

## `develop` Branch (formerly `current`/`master` branch)
This branch has new features or significant changes which are being tested.

Former versioning scheme: `vX.0-CURRENT`
  - The corresponding development branch for version `vX.Y`

Version scheme after `v3.0`:
  1. The name of the latest release version for its development branch
  2. Or `X.Y-CURRENT` for the development branch of upcoming release versions
    - Will be renamed into the real version name upon the official release

Whenever a significant bug fix or change is made in `develop`, the related commits will be merged into `master` branch for the latest release version soon.

- [x] `v1.0-CURRENT`
- [x] `v2.0-CURRENT`
- [x] `v3.0-CURRENT` ( **We are HERE** )

## TODO
- Features from PttBBS to port and test:
    - [x] Compilation tests for data structure size
    - [x] pfterm (4-BSD)
    - [x] BBS-Lua (MIT)
    - [ ] vtuikit (2-BSD)
    - [ ] nios (2-BSD)
    - [ ] logind (2-BSD)
    - [x] WSProxy (MIT)
    - Note: (Maple3) visio = (Pirate/Ptt) term + screen/pfterm + io/(io + nios + vtkbd + vtuikit)
- Features from other Maple3 branches to port and test
    - [x] Screen resizing
    - [ ] BBS-Ruby (MIT; **Testing**)
- [ ] Static Web Page Re-enabling
