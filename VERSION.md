# To-Do and Version Plan

## Conventions of Version Number fields
- `M`: Major version
- `N`: Minor version
- `P`: Patch version
- `Q` and so on: Additional fields
- n consecutive lower-case letter(s) (`m`, `nn`, ...): A n-digit corresponding field
- `x`: A general 1-digit field
- `CODENAME`: A codename field
- `PRERELEASE`: A pre-release field (starts with `alpha`/`beta`/`rc`)

## Legacy Versioning Scheme - `MD.N.P.Q.R` or simply `M.N.P.Q.R`
The versioning scheme originated from MapleBBS.

To referring DreamBBS-specific versions, the letter `D` can be appended to the `M` part. In addition, DreamBBS versions before v4.0.0 can be collectively called `MapleBBS 3D`.

For `M` = `3` and certain cases of `M` = `2`, the abbreviated form `M.np.Q.R` is used, e.g., `3.00` (not `3.0.0`) and `3.02` (not `3.0.2`).

Before the introduction of the 2018 versioning scheme, the latest version name was `3.10 Rev.Beta 3` (inherited from WindTopBBS), *i.e.*:
- `M` = `3`, `N` = `1`, & `P` = `0`.
- `Q` was not actually used but can be viewed as `3`.

Otherwise, `Q` & `R` are reserved for mapping version names in the 2018 versioning scheme before `4D.0.0`/`v4.0.0`.

## The 2018 Versioning Scheme - `vM.N.P` 
Note the presence of `v`.

The versioning scheme introduced in 2018, now deprecated.

Each release using the 2018 versioning scheme has a corresponding legacy version number as well:
- For `M` ≤ `3`, `vM.N.P` corresponding to `3D.xy.N.P`, where each possible `M` maps to a unique `xy`.

For DreamBBS v3, the form `vM.n.P` is used, e.g., `v3.0` (not `v3.00`) and `v3.2` (not `v3.02`). The next major release after DreamBBS v3.3 will be DreamBBS v4.0, so that `N` will not be larger than `9`.

It is acceptable to omit the `v` prefix for DreamBBS because using `M.N.P` along actually does not cause ambiguities:
- DreamBBS derived from MapleBBS not before MapleBBS 2.36, so before DreamBBS v3 (i.e., `M` < `3`), `DreamBBS M.N` refers to `vM.N` but not `MD.N`
- For the 2nd field, DreamBBS v3 allows only 1 digit while MapleBBS 3 allows only 2 digits, so `DreamBBS 3.nn` refers to `3D.nn` but not `v3.nn`, while `DreamBBS 3.n` refers to `v3.n` but not `3D.n`.

However, to avoid confusions, these 2 schemes can be explicitly differentiated by using `MD.N.P` vs. `vM.N.P`.

The version number `M.N.P` will be consistent with the legacy versioning scheme for `4D.0.0`/`v4.0.0` and on.

## Branch/Tag Series
The development workflow introduced in 2018 had led to the two-branch series: RELEASE (formerly TESTING) and CURRENT, which are explained in the following paragraphs.

From v3.0 and on, no dedicated git branches for RELEASE will be created. Instead, new releases on RELEASE are published based on git tags from the `master` branch.

The stages of the overall development workflow after v3.0, from the most unstable to the most stable:
- `<user-or-team>/develop` and PR branches
    - The developer(s)'s develop branch for self-review and peer-review
        - For PR branches, draft PRs are in the self-review stage, while normal PRs are in the peer-review stage.
    - Other names can be used in place of the `develop` at the developer(s)'s intention
    - At the self-review stage, force-updates can be performed at any time at the developer's intention
    - If the commits themselves are checked to not have problems, the peer-review stage should start
    - As soon as the peer-review passes, these commits should be merged onto `master` (fast-forwarding is preferred)
- `master`
    - The branch for commits which passed peer-review
    - Force-updates require acknowledge and agreement from all maintainers in advance
    - Pull requests should target this branch
- `stable`
    - The git tag on `master`, updated when the functionality of `master` is reviewed by the maintainers
- Releases
    - The git tags added on `master` at times
    - The referenced commit should not be changed after the changelog is published

## RELEASE
Before v3.0, RELEASE branches had emphasized more on the stability, the easiness to install, and the ability to deploy on different platform and containers than CURRENT.

These goals have now been transferred to the CURRENT tag `stable`, which is the recommended version for other BBS sites to use.

Versioning Scheme: `vM.N.P`, or just `M.N.P` after `4.0.0`
  - `M`: Major version; indicates significant changes
  - `N`: Minor version; indicates introduction of new features
  - `P`: Patch version; indicates bug fixes or security improvement; based on the previous version
  - `CODENAME` can be omitted for simplicity
    - If `CODENAME` is assigned to a minor version, one of `N` and `CODENAME` can be omitted
  - The present of `PRERELEASE` may indicate potential instability
  - `v0.N` refers to the series of `v0.N.P` versions
  - For `M` > 0, `vM.0` refers to the series of `vM.N.P` versions

### [`v0.95`/`3.10 REV-BETA4`](https://github.com/ccns/dreambbs/tree/v0.95-testing) (`3D.10.95`)
The 2018 versioning scheme is introduced in this minor version.
- [`v0.95.0`/`3.10 REV-BETA4.0`](https://github.com/ccns/dreambbs/releases/tag/v0.95)
- [`v0.95.1`/`3.10 REV-BETA4.1`](https://github.com/ccns/dreambbs/releases/tag/v0.95.1) 
- [`v0.95.2`/`3.10 REV-BETA4.2`](https://github.com/ccns/dreambbs/releases/tag/v0.95.2)
- [`v0.95.3`/`3.10 REV-BETA4.3`](https://github.com/ccns/dreambbs/releases/tag/v0.95.3)
- [`v0.95.4`/`3.10 REV-BETA4.4`](https://github.com/ccns/dreambbs/releases/tag/v0.95.4): Planned to be the last release of `v0.95`

It has become end-of-life when `v0.97.0` was released.

### [`v0.96`](https://github.com/ccns/dreambbs/tree/v0.96-testing) (`3D.10.96`)
The `REV-BETA` part in the version name is no longer used.
- [`v0.96.0`](https://github.com/ccns/dreambbs/releases/tag/v0.96)
- [`v0.96.1`](https://github.com/ccns/dreambbs/releases/tag/v0.96.1)
- [`v0.96.2`](https://github.com/ccns/dreambbs/releases/tag/v0.96.2)
- [`v0.96.3`](https://github.com/ccns/dreambbs/releases/tag/v0.96.3)

Features:
- Sorted common library and refined structure.
- Sample files are moved to another repository.
- Other detailed changes from CURRENT.

It has become end-of-life when `v0.97.1` was released.

### [`v0.97-stratosphere`](https://github.com/ccns/dreambbs/tree/v0.97-stratosphere) (`3D.10.97`)
- [`v0.97-RCx`](https://github.com/ccns/dreambbs/releases/tag/v0.97.0-RC1)-[`v0.97.0`](https://github.com/ccns/dreambbs/releases/tag/v0.97.0): Pre-release
- [`v0.97.1`](https://github.com/ccns/dreambbs/releases/tag/v0.97.1)
- [`v0.97.2`](https://github.com/ccns/dreambbs/releases/tag/v0.97.2)
- [`v0.97.3`](https://github.com/ccns/dreambbs/releases/tag/v0.97.3)

Features:
  - Simple code test utilities.
  - Refined layout: Trailing whitespaces are removed.

### `v0.98`-`v0.99` (`3D.10.98`-`3D.10.99`)
The plans have been dropped and no dedicated branches were created due to significant changes which require comprehensive tests.

Pre-releases for `v1.0` were planned instead:
  - [`v1.0-alpha1`](https://github.com/ccns/dreambbs/releases/tag/v1.0-alpha1)
  - [`v1.0-alpha2`](https://github.com/ccns/dreambbs/releases/tag/v1.0-alpha2)
  - [`v1.0-alpha3`](https://github.com/ccns/dreambbs/releases/tag/v1.0-alpha3)
  - [`v1.0-beta1`](https://github.com/ccns/dreambbs/releases/tag/v1.0-beta1)
  - [`v1.0-RC1`](https://github.com/ccns/dreambbs/releases/tag/v1.0-rc1)
  - [`v1.0-RC2`](https://github.com/ccns/dreambbs/releases/tag/v1.0-rc2)
  - [`v1.0-RC3`](https://github.com/ccns/dreambbs/releases/tag/v1.0-rc3)

### [`v1.N`](https://github.com/ccns/dreambbs/tree/v1.0-rimuru): `Tensure` series (`3D.11.N`)
- [`v1.0.0-rimuru`](https://github.com/ccns/dreambbs/releases/tag/v1.0.0)
- [`v1.1.0-milim`](https://github.com/ccns/dreambbs/releases/tag/v1.1.0): Backports and improvement from CURRENT
  - Features:
    - [x] Screen resizing feature.
- [`v1.1.1`](https://github.com/ccns/dreambbs/releases/tag/v1.1.1)
  - Features:
    - [x] `pfterm`, which was ported in this release version.
- [`v1.2.0-shizu`](https://github.com/ccns/dreambbs/releases/tag/v1.2.0)
- [`v1.3.0-veldora`](https://github.com/ccns/dreambbs/releases/tag/v1.3.0)

### `v2.N`: `Fate` series (`3D.12.N`)
Unlike previous releases, dedicated branches are created per minor version for this major version.
- [`v2.0`](https://github.com/ccns/dreambbs/tree/v2.0-artoria)
  - [`v2.0.0-artoria`](https://github.com/ccns/dreambbs/releases/tag/v2.0.0)
    - Features in plan:
      - [x] Option for disabling DSO.
      - [x] BBS-Lua/BBS-Ruby. (Delayed to `v2.1`)
      - [x] SHA-256-encrypted password support.
      - [x] Preliminary WebSocket support, based on relevant patches to Maple3 and PttBBS.
- [`v2.1`](https://github.com/ccns/dreambbs/tree/v2.1-gilgamesh) (never diverged from CURRENT)
  - [`v2.1-gilgamesh-rc1`](https://github.com/ccns/dreambbs/releases/tag/v2.1-rc1) (pre-release only)
      - Features:
        - Support building with CMake

### `v3.N`; `Four Symbol` series (`3D.21.N`) ( Latest Release Series )
This major version is also named `DreamBBS-202X vN`. `202X` refers to the 2020s decade.

This will be the last major version whose version number is different from the legacy version number.

Due to the shortage of development resources, from this major version and on, only the latest release version is officially maintained.

All the previous branches have become end-of-life when `v3.0` was released.

- [x] [`v3.0`/`202X v0-Azure`](https://github.com/ccns/dreambbs/releases/tag/v3.0.0)
  - [x] Hot-swapping of DSO
  - [x] Force using 32-bit int type structure member in native 64-bit environment
  - [x] Full IPv6 support
  - [x] Auto screen resizing
  - [x] Server-side DBCS character detection
- [x] [`v3.0.1`/`202X v0.1`](https://github.com/ccns/dreambbs/releases/tag/v3.0.1)
  - [x] Improved high resolution login stat chart
- [x] [`v3.1`/`3D-AO.21.1-VRMiliO`](https://github.com/ccns/dreambbs/releases/tag/v3.1.0) (released on `iid/develop`)
  - [x] Provide an option to remove the logout option from the main menu
- [x] [`v3.2`/`202X v2-Shiyuu`](https://github.com/ccns/dreambbs/releases/tag/v3.2.0) (released on `iid/develop`)
  - [x] Initial support of color theme (dark/bright)
- [ ] `v3.3`; planned to be the last release of `v3.X`

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

### `v4.N`/`4D.N`; `Dream` series ( Planned Release Series )
This major version is also named `MapleBBS 4D` by DreamBBS developers to reflect its goals.

Goals:
- Architecture
    - [ ] Rust (allow C/C++ modules)
    - [ ] Unicode + UTF-8 (with legacy contents unaffected)
    - [ ] I18n & l10n
    - [ ] 64-bit `libc::time_t`
    - [ ] Compile once, create sites anywhere
    - [ ] The disappearance of reinvented wheels
    - [ ] Existent *de facto* standard BBS API
    - [ ] Data control logics which isolated from the UI
    - [ ] BBS-Lua 4D, for script-ized layout logics and non-resource-intensive auxiliary functionalities
- Backward Compatibility
    - [ ] Core functionality near-parity of both WindTopBBS and MapleBBS 3.10-itoc combined
    - [ ] Automated upgrade of persistent data and site configuration from the fore-mentioned BBS systems
- UI
    - [ ] Telnet-based TUI targeting XTerm + SSH connection
    - [ ] Resembling elements from the original MapleBBS 3.10 with enhancements
    - [ ] Ready for time-variant terminal size

## CURRENT
Before v3.0, CURRENT had had new features or significant changes which were being tested.

CURRENT now consists of the `master` branch and the tag `stable`. The above goals have now been transferred to the branch `master`.

Former versioning scheme: `vM.0-CURRENT`
  - The corresponding CURRENT version for `vM.N`

Version scheme after `v3.0`:
  1. The name of the latest release version of its release series
  2. Or `M.N-CURRENT` for the CURRENT version of the upcoming release series
    - Will be renamed into the real version name upon the official release

Before v3.0, whenever a significant bug fix or change was made in CURRENT, the related commits would have been soon merged into the dedicated RELEASE branch for the latest release version.

From v3.0 and on, a new release will be published soon instead. For less significant fixes or changes, the tag `stable` will be updated soon instead.

## Special Editions

Sometimes, special editions are released on special events.

### April Fools' Day Event
- [x] 2020: [`v4.20-alpha`](https://github.com/ccns/dreambbs/tree/v4.20-alpha) (Based on `v3.0-CURRENT`)
    - The main menu and the popup menu are movable
    - Every item in the main menu has a verbose explanation
- [x] 2021: [`v4.20-beta`](https://github.com/ccns/dreambbs/tree/v4.20-beta) (Based on `v3.0`/`202X v0`)
    - The main menu and the Xover list have 2 cursors
    - The main menu and the popup menu are movable again
- [x] 2022: [`v4.20-rc.0`](https://github.com/ccns/dreambbs/tree/v4.20-rc.0) (Based on `v3.1-rc.1`/`202X v1-rc.1`)
    - All the previous v4.20 features are back (except for main menu explanations).
    - The popup menu also has 2 cursors.
    - Planned to be integrated into `master` branch.
- [x] 2023: [`v4.20-DAO`](https://github.com/ccns/dreambbs/tree/v4.20-DAO) (`4D-AO.20`) (Based on `v3.1` and `v4.20-rc.0`)
    - All the previous v4.20 features are now disabled by default and can be toggled with `KEY_KONAMI`.
- [x] 2024: [`v4.20-X`](https://github.com/ccns/dreambbs/tree/v4.20-X) (Based on `v4.20-DAO`)
    - Make displayed site name randomly selected from a list if defined
- [x] 2025: [`v3.2.0`](https://github.com/ccns/dreambbs/tree/v3.2.0) (Based on `v4.20-X`)
    - All the previous v4.20 features are incorporated into the regular release.
    - The new experimental bright mode can be toggled with `KEY_SHIYUU`.

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
