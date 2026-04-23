# Submission Checklist

This file is here so the final submission does not become a panic session.

## Build and run checklist

- terminal target builds cleanly
- GUI bridge builds cleanly
- Tkinter GUI starts and talks to the bridge
- startup commands work
- play commands work
- quit paths work

## Repository cleanup checklist

- generated binaries are not tracked
- `build/` and CLion files stay ignored
- test-only scratch files are not mixed into the final submission by accident
- required PDFs and docs are still present

## Deliverable checklist

- terminal version included
- GUI version included
- shared backend included
- report included
- manual tests included if useful for evidence
- build/run instructions included

## Final sanity pass

- rerun manual test list
- rerun strict warning build
- rerun sanitizer check if we changed memory-heavy code
- make one final backup copy outside the working folder
