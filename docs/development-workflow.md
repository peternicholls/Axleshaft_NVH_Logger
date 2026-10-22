# Development workflow

`VERSION` is the single repository version source. Binary format versions and firmware build identity are separate: document and store them when implemented. Never label an old recording with a new format version.

## Each deliverable

- Inspect Git status and preserve unrelated work.
- Keep changes bounded; use descriptive `codex/` branches for substantive implementation and integrate verified slices into `main`.
- Update progress as milestones change and record blocked or unverified items explicitly.
- Increment the repository version: patch for compatible fixes/documentation/tooling, minor for a new stage capability or incompatible interface/format. Initial documentation establishes 0.1.0; the next documentation fix would be 0.1.1.
- Add a matching dated changelog entry and development-log evidence. Include bookkeeping in the same delivery commit.
- Run checks appropriate to the change, inspect results and distinguish automated validation from hardware acceptance.
- Use Lore commit messages: why first, context next, then useful native trailers separated by a blank line. Include verification gaps honestly.

No automatic tags or remote publication are configured. Only create a release tag for an intentionally accepted release, after checking the working tree and evidence.

## Evidence records

For hardware runs record date, repository version and commit, board/core version, wiring, card identity, test duration, serial output location, recording identity and validator output. Mark missing fields unknown. Keep raw recordings local by default; add small deliberate fixtures explicitly if needed.

Documentation-only deliverables need link, version-alignment and whitespace checks; they do not need hardware tests. Firmware changes need compilation and appropriate host checks plus explicit pending bench gates when hardware is unavailable.
