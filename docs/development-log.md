# Development log

## 2026-10-22 — 0.1.0: repository foundation

Initial baseline commit: f9b294a. Preserved the supplied handover, Stage 0A sketch and its README without edits. Established `main`; ignored local runtime/editor artifacts and raw recordings. No remote configured.

Added documentation index, stage-gated development plan, progress tracker, wiring/test references and format/timing design placeholders. Established a mandatory version, changelog, progress and evidence update for each deliverable in the repository working contract.

Validation: local relative Markdown links, required documentation files, version/changelog alignment, original-file preservation and Git whitespace checks passed. Firmware has not been compiled, uploaded or rerun. Historical Stage 0A hardware results remain attributed to the handover. Stage 0B implementation and all subsequent hardware gates remain pending.

## 2026-10-22 — 0.1.1: Stage 0B planning

Created `docs/stage0b-plan.md` with a local planning copy under ignored `.omx/plans/`. Defined producer/storage separation, bounded buffering, exact loss accounting, deterministic records, control states, ordered deliverables and host/build/bench gates. Recorded the imported sketch's `no/*` prefix and available Arduino CLI without modifying firmware or assuming a valid installed target.

Validation: relative documentation links, 0.1.1 bookkeeping alignment, local plan-copy consistency and whitespace checks passed. No firmware compilation, API qualification, upload or hardware measurements performed. Stage 0B implementation and acceptance remain pending.

## 2026-10-22 — 0.1.2: standalone stage packaging

Updated Stage 0B planning to follow the user's Stage 0A folder convention. `Stage0B_Recorder/` will contain the sketch, README.txt, required local helpers and standalone validator, with no sibling runtime dependencies. Tests remain repository-level and use the bundled validator directly. No empty stage folder or placeholder firmware created.

Validation: documentation links, version alignment, local plan-copy consistency and whitespace checks passed. Implementation and hardware acceptance remain pending.
