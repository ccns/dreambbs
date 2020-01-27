# To-Do and Version Plan

- **Why we do this: For fun.**

- Finished features/releases are ~`stroke through`~.

- The previous Version Name: (`WindTop 3.10`) `Rev.Beta 3`

- Current Version Branches:
    - `testing` (release branch): Emphasize more on the stability, the easiness to install, and the ability to deploy on different platform and containers, than the `current` version branch.
        - Naming convention: `vX.Y.Z`
            - `X`: Major version; indicates significant changes
            - `Y`: Minor version; indicates introduction of new features
            - `Z`: Patch version; indicates bug fixes or security improvement; based on the previous version
        - `v0.Y` refers to both the `v0.Y.0` version and the branch for `v0.Y.Z` versions
        - For `X`>0, `vX.0` refers to both the `vX.0.0` version and the branch for `vX.Y.Z` versions
    - `current` (`master`/develop branch): Has new features or significant changes which are being tested.
        - Naming convention: `vX.0-CURRENT`: Develop branch for version `vX.Y`
        - Whenever a significant bug fix or change is made in `current`, the related commits will be ported back to released `testing` branches.

- Planned version names for `testing` (releases):

    - `v0.95`: Aka. `REV-BETA4`. The naming style is changed. ( **End-of-Life** )
        - `v0.95.1`: Aka. `REV-BETA4.1`
        - `v0.95.2`: Aka. `REV-BETA4.2`
        - `v0.95.3`: Aka. `REV-BETA4.3`
        - `v0.95.4`: Aka. `REV-BETA4.4`; planned to be the final release of `v0.95-testing`
        - It will become end-of-life when `v0.97.0` is released.

    - `v0.96`: Dropping the `REV-BETA` name, the version name became `DreamBBS Version 0.96`. ( **End-of-Life** )
        - Features: 
          - Sorted common library and refined structure.
          - Sample files are moved to another repository.
          - Other detailed changes from `current` version.
        - `v0.96.1`
        - `v0.96.2`
        - It will become end-of-life when `v0.97.1` is released.

    - `v0.97`: Version 0.97; codename `stratosphere` ( **End-of-Life** )
        - `v0.97-RCx`~`v0.97.0`: Pre-release
        - `v0.97.1`
        - `v0.97.2`
        - `v0.97.3`
        - Features:
          - Simple code test utilities.
          - Refined layout: Trailing whitespaces are removed.
    - Drop plans for `v0.98`~`v0.99` due to big changes which will be tested for a long time.
        - Alternative planned names:
          - ~`v1.0-alpha1`~, ~`v1.0-alpha2`~, ~`v1.0-alpha3`~, ~`v1.0-beta1`~, ~`v1.0-RC1`~, ~`v1.0-RC2`~, ~`v1.0-RC3`~
    - `v1.0`: Version 1.X; codename `rimuru`
        - `v1.1.0`: Backports and improvement from `current`; codename `milim`
        - Features:
          - ~~Screen resizing feature.~~
        - `v1.1.1`
        - Features:
          - ~~`pfterm`, which is ported in this release version.~~
          - Others: Please see <https://github.com/ccns/dreambbs/releases/tag/v1.1.0>.
        - `v1.2.0`: Codename `shizu`
        - `v1.3.0`: Codename `veldora`
    - `v2.0`: Version 2.X; codename `artoria` ( **Latest Release Branch** )
        - Features in plan:
          - ~~Option for disabling DSO.~~
          - BBS-Lua/BBS-Ruby.
          - ~~SHA-256-encrypted password support.~~
          - ~~Preliminary WebSocket support, based on relevant patches to Maple3 and PttBBS.~~
        - `v2.1.0`: Codename `gilgamesh`
        - Features:
          - Support building with CMake
    - `v3.0 ~`: Version 3.X or later
        - Features in plan:
          - Support forcing use 32-bit int type variables in 64-bit environment when BBS is compiled natively.
- Planned version names for `current` (`master` branch):
    - `v1.0-CURRENT`
    - `v2.0-CURRENT`
    - `v3.0-CURRENT` ( **We are HERE** )
    - `vX.0-CURRENT`


## TODO
- To port and test from PttBBS
    - ~variable size test~
    - ~pfterm~ ~port~ ~test~ (4-BSD)
    - ~BBS-Lua~ ~port~ ~test~ (MIT)
    - vtuikit port test (2-BSD)
    - nios port test (2-BSD)
    - logind port test (2-BSD)
    - WSProxy ~port~ test (MIT; **Testing**)
- Other (improved) features from other Maple3 versions
    - BBS-Ruby ~port~ test (**Testing**)
    - ~Screen resizing feature~
- Static Web Page Re-enabling
