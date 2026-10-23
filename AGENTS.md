# Project working contract

Follow the session's supplied operating instructions, including the Lore commit protocol.

## Required bookkeeping

For every coherent deliverable that changes tracked code, documentation, tests or tooling:

1. Update `docs/progress.md` with current stage, completed work, remaining work and evidence. Update it during substantive milestones, not only at final delivery.
2. Increment `VERSION` once per deliverable, before its delivery commit. Use semantic versioning while pre-1.0: patch for compatible fixes/docs/tooling, minor for a new stage capability or incompatible format/API change. Reserve 1.0.0 for an explicitly accepted integrated release.
3. Add a dated entry under the matching version in `CHANGELOG.md` and a concise entry in `docs/development-log.md` describing validation and limitations.
4. Run relevant checks and record actual results. Distinguish host checks, firmware compilation and hardware acceptance. Never mark a stage accepted without its required hardware evidence.
5. Commit the implementation and its bookkeeping together. Intermediate work-in-progress commits may share the pending deliverable version; no completed deliverable may omit its version/changelog/progress updates.

Keep `handover_v1.md` and the original Stage 0A sketch as historical references. Preserve CS=13, SCK=18, MISO=19, MOSI=23 and 1 MHz SPI unless measured evidence justifies a change.

User explicitly authorised advancing standalone Stage 1 and, on 2026-10-23, GY-521 setup/development followed by a marked-shaft mechanical-tap synchronisation bench despite outstanding Stage 0B checks. Keep those checks open. Qualify standalone IMU acquisition and rig optical detection before combined bench captures; this does not imply vehicle or release acceptance. Do not invent missing results, package versions or hardware measurements. No new dependencies without explicit user request.

## Stage packaging

Deliver Stage 0B in root-level `Stage0B_Recorder/`, matching `Stage0A_SD_Test/`: sketch named after the folder, `README.txt`, all required local firmware helpers and standalone recording validator. The folder must be independently copyable and usable, with wiring, build/upload, operation and validation instructions. Do not nest it under `firmware/` or require sibling repository files for operation. Keep project progress and development evidence in `docs/`.

## Commit messages

Use an intent line explaining why, with narrative context and useful native trailers such as `Tested:`, `Not-tested:`, `Confidence:` and `Scope-risk:`. See `docs/development-workflow.md` for delivery rules.

## Upload responsibility

This chat prepares code, instructions, plans and local verification only. Do not upload firmware, operate physical hardware or claim fresh physical acceptance here. Upload/capture is performed in the user's separate workflow; consume its saved reports as evidence.
