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

Complete Stage 0B acceptance before starting Stage 1 or Stage 2. Do not invent missing results, package versions or hardware measurements. No new dependencies without explicit user request.

## Commit messages

Use an intent line explaining why, with narrative context and useful native trailers such as `Tested:`, `Not-tested:`, `Confidence:` and `Scope-risk:`. See `docs/development-workflow.md` for delivery rules.
