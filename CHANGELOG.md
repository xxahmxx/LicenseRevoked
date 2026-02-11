# Changelog - License Revoked

All notable changes to this project will be documented in this file.

## [1.2.0.x] - 2026-02-11
### Fixed
- **Stability:** Fixed a critical breaking issue that occurred after the "Game Over" sequence was triggered.
- **Rendering:** Eliminated screen flickering when closing the game by implementing thread synchronization and event-driven shutdown logic.
- **Alt+Tab Support:** Fixed issues with the overlay appearing on the desktop when the game was minimized.

## [1.1.0] - 2026-02-10
### Added
- **Configurability:** Introduced `.ini` file support for user customization.
- **Custom Penalties:** Added ability to configure specific point penalties for different offenses.
- **Gameplay Parameters:** Users can now define the maximum point limit and the number of days required for a clean record reset.
- **Localization:** Added multi-language support (English and Polish) selectable via configuration.

## [1.0.0] - 2026-02-09
### Added
- **Initial Release:** Core plugin functionality featuring the penalty point system and input locking.
- **Profile Persistence:** Implemented a unique save system that links penalty points to specific driver profiles.
- **Native Overlay:** Introduced a DX11-based UI to display notifications and legal notices.