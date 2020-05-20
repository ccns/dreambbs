# To-Do and Version Plan

**Why do we do this? For fun!**

## Legacy Versioning Scheme - `DreamBBS X.Y.Z`; abbreviated as `X.Y.Z` 
The versioning scheme originated from MapleBBS.

Before the introduction of the new versioning scheme, the latest version name was `WindTop 3.10 Rev.Beta 3` or `DreamBBS 3.10 Rev.Beta 3`.

## Current Versioning Scheme - `DreamBBS vX.Y.Z`; abbreviated as `vX.Y.Z` 
Note the presence of `v`.

Each new version has a corresponding legacy version number as well.

## Branches of Development
The new development workflow has led to the two main branches seen today: `testing` and `current`.

## `testing` Branch (Release Branch)
This branch emphasize more on the stability, the easiness to install, and the ability to deploy on different platform and containers, than the `current` branch.

Versioning Scheme: `vX.Y.Z`
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
- `v0.95.4`: Aka. `3.10 REV-BETA4.4`; planned to be the final release of `v0.95`

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
Plans dropped due to big changes which will be tested for a long time.
Planned alternative names:
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

### `v3.X`: Aka. `2020 vX`; `Four Symbol` series (`3.20.X`) ( Upcoming Release Branch )
The full name of this major version is `DreamBBS-2020 vX`.

This will be the last major version whose version number is different from the legacy version number.

- `2020 v0` (`v3.0`)
- `2020 v1` (`v3.1`)
- `2020 v2` (`v3.2`)
- `2020 v3` (`v3.3`); planned to be the final release of `v3.X`

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

## `current` Branch (`master`/Development Branch)
This branch has new features or significant changes which are being tested.

Versioning scheme: `vX.0-CURRENT`
  - The corresponding development branch for version `vX.Y`

Whenever a significant bug fix or change is made in `current`, the related commits will be ported back to released `testing` branches.

- `v1.0-CURRENT`
- `v2.0-CURRENT`
- `v3.0-CURRENT` ( **We are HERE** )
- `vX.0-CURRENT`

## TODO
- Features from PttBBS to port and test:
    - [x] Compilation tests for data structure size
    - [x] pfterm (4-BSD)
    - [x] BBS-Lua (MIT)
    - [ ] vtuikit (2-BSD)
    - [ ] nios (2-BSD)
    - [ ] logind (2-BSD)
    - [ ] WSProxy (MIT; **Testing**)
    - Note: (Maple3) visio = (Pirate/Ptt) term + screen/pfterm + io/(io + nios + vtkbd + vtuikit)
- Features from other Maple3 branches to port and test
    - [x] Screen resizing
    - [ ] BBS-Ruby (MIT; **Testing**)
- [ ] Static Web Page Re-enabling
