# CSA02 – Assignment 4: Modelling Real-World Data with Structures and Unions

- **Student name:** <YOUR NAME>
- **Register number:** <YOUR REGISTER NUMBER>
- **Compiler:** gcc 13.3.0 (Ubuntu 24.04), standard C (gnu17 default)
- **Compile command (every program):** `gcc -Wall -Wextra -o <exe> <file.c>`
  (two-file Task 5 demo: `gcc -Wall -Wextra -o extern_demo Task05/extern_main.c Task05/extern_counter.c`)
- **Build & run everything:** `./run_all.sh` (writes `Taskxx/compile_*.txt` / `output_*.txt` and the matching `.png` screenshots via `make_screenshots.py`)
- **Report:** `Report/Assignment4_Report.docx` (also `.md`)

## Tasks and results
| Task | Run | Result |
|---|---|---|
| 01 Patient register redesign | `./Task01/patient` | Nested structs, `addPatient(struct patient_register*, const struct patient*)`; invalid/full cases rejected; `sizeof(struct patient)=88`. |
| 02 Courier parcels | `./Task02/parcel`, `./Task02/parcel_full` | TODO 1 & 2 completed; output matches expected; nested vs flat both 140 B. |
| 03 Weather packets | `./Task03/sensor`, `./Task03/sensor_extra` | `sizeof union = 8, struct reading = 12`; output matches; misuse gives 1107034112. |
| 04 Structure forensics | `Task04/buggy.c` (fails to compile), `evidence`, `fixed`, `byvalue_alt` | Pass-by-value copy + `p.` vs `p->` fixed; address evidence recorded. |
| 05 Storage classes | `token`, `order`, `extern_demo`, `register_loop`, `two_counters`, `storage_demo` | static 1001..1003 vs auto 1001 x3; extern across 2 files; register `&` rejected by compiler. |
| 06 Canteen | `./Task06/canteen` | Tagged union items, safe stock update, expiry warning, revenue-sorted summary. |

## Screenshots
Every program's compile result and run output is saved as a PNG terminal screenshot in its `TaskXX/` folder (e.g. `Task01/output_patient.png`, `Task01/compile_patient.png`). Regenerate with `python3 make_screenshots.py`.

## Git commits (make at least one per task)
```
git init && git add Task01 && git commit -m "Task 1: patient register redesign"
git add Task02 && git commit -m "Task 2: parcel structures"
... (one commit per task, then README/report)
```
