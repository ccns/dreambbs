# To-Do and Version Plan

## Conventions of Version Number fields
- `M`: Major version
- `N`: Minor version
- `P`: Patch version
- `Q` and so on: Additional fields
- n consecutive lower case letter(s) (`m`, `nn`, ...): A n-digit corresponding field
- `x`: A general 1-digit field

## Legacy Versioning Scheme - `MD.N.P.Q` or simply `M.N.P.Q`
The versioning scheme originated from MapleBBS.

To referring DreamBBS-specific versions, the letter `D` can be appended to the `M` part. In addition, DreamBBS versions before v4.0.0 can be collectively called `MapleBBS 3D`.

For MapleBBS 3, the form `3.nn.P.Q` is used, e.g., `3.00` (not `3.0`) and `3.02` (not `3.2`).
- It is acceptable to view these two digits as `np`: `N` is `0` and `P` is `2`, and thus the legacy versioning scheme for MapleBBS 3 is instead `3.np.Q.R`.
    However, this view is not adopted in this article.

Before the introduction of the 2018 versioning scheme, the latest version name was `3.10 Rev.Beta 3` (inherited from WindTopBBS), and the `.P` part was not actually used.
- However, it is acceptable to view `Rev.Beta 3` as the `.P` part (`3`).

The `.Q` part is reserved for mapping version names in the current versioning scheme before `4D.0.0`/`v4.0.0`.

## The 2018 Versioning Scheme - `vM.N.P` 
Note the presence of `v`.

The versioning scheme introduced in 2018, now deprecated.

Each release using the 2018 versioning scheme has a corresponding legacy version number as well:
- For `M` ≤ `3`, `vM.N.P` corresponding to `3D.X.N.P`, where each possible `M` maps to a unique `X`.

For DreamBBS v3, the form `vM.n.P` is used, e.g., `v3.0` (not `v3.00`) and `v3.2` (not `v3.02`). The next major release after DreamBBS v3.3 will be DreamBBS v4.0, so that `N` will not be larger than `9`.

It is acceptable to omit the `v` prefix for DreamBBS because using `M.N.P` along actually does not cause ambiguities:
- DreamBBS derived from MapleBBS not before MapleBBS 2.36, so before DreamBBS v3 (i.e., `M` < `3`), `DreamBBS M.N` refers to `vM.N` but not `MD.N`
- DreamBBS v3 and MapleBBS 3 use different numbers of digits for `N`, so `DreamBBS 3.nn` refers to `3D.nn` but not `v3.nn`, while `DreamBBS 3.n` refers to `v3.n` but not `3D.n`.

However, to avoid confusions, these 2 schemes can be explicitly differentiated by using `MD.N.P` vs. `vM.N.P`.

The version number `M.N.P` will be consistent with the legacy versioning scheme for `4D.0.0`/`v4.0.0` and on.

## Branches of Development
The new development workflow has led to the two main branches seen today: Release and `current`, which are explained in the following paragraphs.

From v3.0 and on, no dedicated git branches for releases will be created. Instead, new releases on the release development branch are published based on git tags from the `master` branch.

After v3.0, there are unstable branches for the convenience of testing as well: `develop` and `<user>/develop`.

The stages of the overall development workflow after v3.0, from the most unstable to the most stable:
- `<user>/develop`
    - The developer's develop branch for self-review
    - Other names can be used in place of the `develop` at the developer's intention
    - Force-updates can be performed at any time at the developer's intention
    - As soon as the commits themselves are checked to not have problems, these commits should be merged onto `develop` (fast-forwarding is preferred)
- `develop`
    - The branch for commits which need review from the maintainers
    - Force-updates require acknowledge and agreement from all maintainers in advance
    - As soon as the review are done, the commits should be merged onto `master` (fast-forwarding is preferred)
    - Pull requests should target this branch
- `master`
    - The stable branch where the functionality are reviewed by the maintainers
    - Force-updates should not be performed on this branch
- Releases
    - The git tags added on `master` at times
    - The referenced commit should not be changed after the changelog is published

## Release Branch (formerly `testing`)
This branch emphasize more on the stability, the easiness to install, and the ability to deploy on different platform and containers, than the `current` branch.

Versioning Scheme: `vM.N.P`, or just `M.N.P` after `4.0.0`
  - `M`: Major version; indicates significant changes
  - `N`: Minor version; indicates introduction of new features
  - `P`: Patch version; indicates bug fixes or security improvement; based on the previous version
  - `v0.N` refers to the branch for `v0.N.P` versions
  - For `M` > 0, `vM.0` refers to the branch for `vM.N.P` versions

### [`v0.95`](https://github.com/ccns/dreambbs/releases/tag/v0.95): Aka. `3.10 REV-BETA4` (`3D.10.95`) ( **End-of-Life** )
The 2018 versioning scheme is introduced in this minor version.
- [`v0.95.1`](https://github.com/ccns/dreambbs/releases/tag/v0.95.1): Aka. `3.10 REV-BETA4.1`
- [`v0.95.2`](https://github.com/ccns/dreambbs/releases/tag/v0.95.2): Aka. `3.10 REV-BETA4.2`
- [`v0.95.3`](https://github.com/ccns/dreambbs/releases/tag/v0.95.3): Aka. `3.10 REV-BETA4.3`
- [`v0.95.4`](https://github.com/ccns/dreambbs/releases/tag/v0.95.4): Aka. `3.10 REV-BETA4.4`; planned to be the last release of `v0.95`

It has become end-of-life when `v0.97.0` was released.

### [`v0.96`](https://github.com/ccns/dreambbs/releases/tag/v0.96) (`3D.10.96`) ( **End-of-Life** )
The `REV-BETA` part in the version name is no longer used.
- [`v0.96.1`](https://github.com/ccns/dreambbs/releases/tag/v0.96.1)
- [`v0.96.2`](https://github.com/ccns/dreambbs/releases/tag/v0.96.2)
- [`v0.96.3`](https://github.com/ccns/dreambbs/releases/tag/v0.96.3)

Features:
- Sorted common library and refined structure.
- Sample files are moved to another repository.
- Other detailed changes from `current` version.

It has become end-of-life when `v0.97.1` was released.

### `v0.97`: `stratosphere` (`3D.10.97`) ( **End-of-Life** )
- [`v0.97-RCx`](https://github.com/ccns/dreambbs/releases/tag/v0.97.0-RC1)-[`v0.97.0`](https://github.com/ccns/dreambbs/releases/tag/v0.97.0): Pre-release
- [`v0.97.1`](https://github.com/ccns/dreambbs/releases/tag/v0.97.1)
- [`v0.97.2`](https://github.com/ccns/dreambbs/releases/tag/v0.97.2)
- [`v0.97.3`](https://github.com/ccns/dreambbs/releases/tag/v0.97.3)

Features:
  - Simple code test utilities.
  - Refined layout: Trailing whitespaces are removed.

### `v0.98`-`v0.99` (`3D.10.98`-`3D.10.99`)
The plans have been dropped due to significant changes which require comprehensive tests.

Pre-releases for `v1.0` were planned instead:
  - [`v1.0-alpha1`](https://github.com/ccns/dreambbs/releases/tag/v1.0-alpha1)
  - [`v1.0-alpha2`](https://github.com/ccns/dreambbs/releases/tag/v1.0-alpha2)
  - [`v1.0-alpha3`](https://github.com/ccns/dreambbs/releases/tag/v1.0-alpha3)
  - [`v1.0-beta1`](https://github.com/ccns/dreambbs/releases/tag/v1.0-beta1)
  - [`v1.0-RC1`](https://github.com/ccns/dreambbs/releases/tag/v1.0-rc1)
  - [`v1.0-RC2`](https://github.com/ccns/dreambbs/releases/tag/v1.0-rc2)
  - [`v1.0-RC3`](https://github.com/ccns/dreambbs/releases/tag/v1.0-rc3)

### `v1.N`: `Tensure` series (`3D.11.N`)
- [`v1.0.0`](https://github.com/ccns/dreambbs/releases/tag/v1.0.0): `rimuru`
- [`v1.1.0`](https://github.com/ccns/dreambbs/releases/tag/v1.1.0): `milim`; backports and improvement from `current`
  - Features:
    - [x] Screen resizing feature.
- [`v1.1.1`](https://github.com/ccns/dreambbs/releases/tag/v1.1.1)
  - Features:
    - [x] `pfterm`, which was ported in this release version.
- [`v1.2.0`](https://github.com/ccns/dreambbs/releases/tag/v1.2.0): `shizu`
- [`v1.3.0`](https://github.com/ccns/dreambbs/releases/tag/v1.3.0): `veldora`

### `v2.N`: `Fate` series (`3D.12.N`) ( **Latest Release Branch** )
- [`v2.0.0`](https://github.com/ccns/dreambbs/releases/tag/v2.0.0): `artoria`
  - Features in plan:
    - [x] Option for disabling DSO.
    - [ ] BBS-Lua/BBS-Ruby.
    - [x] SHA-256-encrypted password support.
    - [x] Preliminary WebSocket support, based on relevant patches to Maple3 and PttBBS.
- [`v2.1-rc1`](https://github.com/ccns/dreambbs/releases/tag/v2.1-rc1): `gilgamesh` (pre-release only)
    - Features:
      - Support building with CMake

### `202X vN` (`v3.N`); `Four Symbol` series (`3D.21.N`) ( Upcoming Release Branch )
The full name of this major version is `DreamBBS-202X vN`. `202X` refers to the 2020s decade.

This will be the last major version whose version number is different from the legacy version number.

Due to the shortage of development resources, from this major version and on, only the latest release version is officially maintained.

- [x] [`202X v0` (`v3.0`)](https://github.com/ccns/dreambbs/releases/tag/v3.0.0)
  - [x] Hot-swapping of DSO
  - [x] Force using 32-bit int type structure member in native 64-bit environment
  - [x] Full IPv6 support
  - [x] Auto screen resizing
  - [x] Server-side DBCS character detection
- [x] [`202X v0.1` (`v3.0.1`)](https://github.com/ccns/dreambbs/releases/tag/v3.0.1)
  - [x] Improved high resolution login stat chart
- [ ] `202X v1` (`v3.1`) (**Testing**)
- [ ] `202X v2` (`v3.2`)
- [ ] `202X v3` (`v3.3`); planned to be the last release of `v3.X`

Features in plan:
  - [ ] Built-in Big5-UAO/UTF-8 conversion support.
  - [ ] Data races prevention using atomic variables and `pthread` mutexes.
  - [ ] Replace System V shared memory API calls with POSIX shared memory API.
  - [ ] Support using 64-bit `time_t` to make the BBS system to be immune to the 2038 problem.
    - **48** different structs need to be transformed
      - Not feasible in short-term
  - [ ] Refactor out boilerplate code in xover list system
  - [ ] Merge popup menu systems into main menu system
  - [ ] Merge main menu system into xover list system

## `master` Branch (Current Branch)
This branch has new features or significant changes which are being tested.

Former versioning scheme: `vM.0-CURRENT`
  - The corresponding development branch for version `vM.N`

Version scheme after `v3.0`:
  1. The name of the latest release version for its development branch
  2. Or `M.N-CURRENT` for the development branch of upcoming release versions
    - Will be renamed into the real version name upon the official release

Whenever a significant bug fix or change is made in `current`, the related commits will be merged into `testing` branch for the latest release version soon.

However, from v3.0 and on, instead of merge the commits into the dedicated branch, a new release will be published soon.

- [x] `v1.0-CURRENT`
- [x] `v2.0-CURRENT`
- [x] `v3.0-CURRENT` ( **We are HERE** )

## Special Release Versions

Sometimes, special versions are released on special events.

### April Fools' Day Event
- [x] [`v4.20-alpha`](https://github.com/ccns/dreambbs/tree/v4.20-alpha) (2020) (Based on `v3.0-CURRENT`)
    - The main menu and the popup menu are movable
    - Every items in the main menu have a verbose explanation
- [x] [`v4.20-beta`](https://github.com/ccns/dreambbs/tree/v4.20-beta) (2021) (Based on `202X v0`/`v3.0`)
    - The main menu and the Xover list have 2 cursors
    - The main menu and the popup menu are movable again
- [x] [`v4.20-rc.0`](https://github.com/ccns/dreambbs/tree/v4.20-rc.0) (Based on `202X v1-rc.1/`v3.1-rc.1`)
    - All the previous v4.20 features are back.
    - The popup menu also has 2 cursors.

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
    - [x] BBS-Ruby (MIT; **Enhancing**)
- [ ] Static Web Page Re-enabling
