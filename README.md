# Insurance Policy Management CLI (C++)

> A clean, interview-friendly **C++17** console app to manage **Clients, Policies, and Premium Payments** with text-file persistence.
> Focuses on solid OOP, simple persistence, date handling, and report generation.

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)
![Status](https://img.shields.io/badge/status-stable-brightgreen.svg)

---

## ✨ Highlights

* **OOP design**: `Person → Client`, `Policy`, `Payment` with service layers (`ClientService`, `PolicyService`, `PaymentService`).
* **Persistence**: simple, human-readable text files (`clients.txt`, `policies.txt`, `payments.txt`) using pipe-delimited records.
* **Business logic**: next due date, remaining balance, approximate months paid, policy end date.
* **Reports (polymorphism)**: All Clients, All Policies, Expiring Policies (N months), Clients with Unpaid Premiums.
* **Robust date utilities**: ISO `YYYY-MM-DD`, leap year handling, add-months with end-of-month rules.
* **Portable**: single binary, no external dependencies.

---

## 🧭 Project Structure

```
/ (working directory)
├─ .vscode/ # Editor tasks/settings (optional)
├─ output/ # (Optional) output artifacts you generate
├─ README.md # This file
├─ clients.txt # Data file (pipe-delimited)
├─ insurance.exe # Built binary (one of your Windows builds)
├─ main.cpp # Source code (all-in-one)
├─ main.exe # Built binary (another build output)
├─ payments.txt # Data file (pipe-delimited)
└─ policies.txt # Data file (pipe-delimited)
```

> The application reads/writes the `.txt` files from the **current working directory**.

---

## 🛠 Build & Run

### Prereqs

* A modern C++ compiler:

  * **Linux/macOS**: g++ 9+ or clang++ 10+ (C++17)
  * **Windows**: MSVC (Visual Studio 2019+/Build Tools) or MinGW-w64
* No external libraries required.

### Quick build (single file)

#### Linux / macOS (g++)

```bash
g++ -std=gnu++17 -O2 -Wall -Wextra -o insurance main.cpp
./insurance
```

#### Windows (PowerShell, MSVC)

```powershell
cl /std:c++17 /O2 /EHsc main.cpp /Fe:insurance.exe
.\insurance.exe
```

#### Windows (MinGW-w64)

```bash
g++ -std=gnu++17 -O2 -Wall -Wextra -o insurance.exe main.cpp
insurance.exe
```

> If you organize sources later, you can introduce CMake—this single-file version compiles as shown.

---

## 💾 Data Files & Formats

All three files are **pipe-delimited** (`|`) and created automatically if missing.

### `clients.txt`

```
id|name|age|contact|address
```

**Example**

```
1001|Alice Johnson|31|alice@example.com|Bangalore
1002|Bob Singh|45|+91-98xxx|Mumbai
```

### `policies.txt`

```
policyId|type|monthlyPremium|durationMonths|clientId|startDate(YYYY-MM-DD)
```

**Example**

```
P1001|Health|1500.00|12|1001|2025-01-15
P1002|Life|2200.00|18|1002|2025-03-01
```

### `payments.txt`

```
policyId|amount|date(YYYY-MM-DD)
```

**Example**

```
P1001|1500.00|2025-02-15
P1001|1500.00|2025-03-15
P1002|2200.00|2025-04-01
```

---

## 🖥 App Overview (Menus)

```
==============================
Insurance Policy Management
==============================
1) Client Management
2) Policy Management
3) Premium Payments / Status
4) Reports
0) Exit
> 
```

### 1) Client Management

* Add / View / Search (by ID or name keyword) / Update / Delete
* **Deletion** guarded: cannot delete a client if they still have policies.

### 2) Policy Management

* Add policy (client must exist)
* View all policies
* Search (by PolicyID or ClientID)
* Update fields with optional edits (press ENTER to keep existing)
* Delete policy (blocked if payments exist)

### 3) Premium Payments / Status

* Record payment (free-form amount + date validation)
* Show payment history
* Next due date & remaining balance
* Full policy status (client info, totals, months paid, due date)

### 4) Reports (via polymorphism)

* **All Clients**
* **All Policies**
* **Policies expiring within N months**
* **Clients with unpaid premiums** (based on total due vs total paid)

---

## 🔍 Business Logic (What to explain in interviews)

**Eventual Goals:** Track policy lifecycle, monthly payments, and produce actionable lists (expiring policies, unpaid balances).

1. **Date Handling**

   * `parseDate` validates `YYYY-MM-DD`.
   * `isLeap`, `daysInMonth`, and `addMonths` handle calendar edge cases.
   * **End-of-month rule**: when adding months, days clamp to the target month (e.g., Jan 31 + 1 month → Feb 28/29).

2. **Next Due Date**

   * Compute **months paid** = `floor(totalPaid / monthlyPremium)`.
   * Next due date = `startDate + (monthsPaid + 1)` months.
   * If `monthsPaid >= durationMonths` → policy fully paid (no dues).

3. **Remaining Balance**

   * `remainingBalance = monthlyPremium * durationMonths - totalPaid`, clamped at 0.

4. **Policy End Date**

   * `endDate = startDate + durationMonths`.

5. **Data Integrity & Guards**

   * Cannot delete **Client** if they have **Policies**.
   * Cannot delete **Policy** if it has **Payments**.
   * Minimal numeric validation (`isNumber`), safe defaults if date fails parse (falls back to “today”).

6. **Persistence Layer**

   * Each `*Service` loads on startup, writes on each change:

     * `ClientService` → `clients.txt`
     * `PolicyService` → `policies.txt`
     * `PaymentService` → `payments.txt`
   * Readable text makes debugging and demos simple.

7. **Reports (Polymorphism)**

   * Base class: `Report` with `virtual void generate()`.
   * Implementations:

     * `AllClientsReport`
     * `AllPoliciesReport`
     * `ExpiringPoliciesReport`
     * `UnpaidClientsReport`
   * Extensible without changing calling code.

---

## 🧪 Sample Walkthrough

1. **Create a client**

```
Client Management → Add Client
Name: Alice Johnson
Age: 31
Contact: alice@example.com
Address: Bangalore
[OK] Client added with ID: 1001
```

2. **Create a policy**

```
Policy Management → Add Policy
Client ID: 1001
Type: Health
Monthly Premium: 1500
Duration (months): 12
Policy Start Date (YYYY-MM-DD): 2025-01-15
[OK] Policy created: P1001
```

3. **Record payments**

```
Premium Payments / Status → Record Payment
Policy ID: P1001
Amount: 1500
Payment Date (YYYY-MM-DD): 2025-02-15
[OK] Payment recorded.
```

4. **Check due and balance**

```
Premium Payments / Status → Next Due / Remaining Balance
Policy ID: P1001
== Balance & Due ==
Monthly Premium: 1500
Duration: 12 months
Start: 2025-01-15
Total Due (full term): 18000
Total Paid: 1500
Months Paid (approx): 1 / 12
Next Due Date: 2025-03-15
Remaining Balance: 16500
```

5. **Run a report**

```
Reports → Policies Expiring in Next N Months
Enter N (months): 6
[... lists policies whose end date falls within the next 6 months ...]
```

---

## 🧩 Key Classes (Cheat Sheet)

* **`Person`** → base for shared fields: `name`, `contact`, `address`.
* **`Client : Person`** → adds `id`, `age`, `fromRecord`/`toRecord`.
* **`Policy`** → `policyId`, `type`, `monthlyPremium`, `durationMonths`, `clientId`, `startDate`.
* **`Payment`** → `policyId`, `amount`, `date`.
* **Services**

  * `ClientService` → CRUD + search; prevents deletion if policies exist.
  * `PolicyService` → CRUD; prevents deletion if payments exist; date helpers.
  * `PaymentService` → append/aggregate payments.
* **Reports (polymorphic)** → printing tabular summaries with `setw`, `left`.

---

## 🧰 Implementation Notes

* **C++17**: Uses structured initialization of `Date`, lambda helpers, and standard containers/algorithms.
* **I/O**: Synchronous file writes for simplicity; files overwrite on save for deterministic state.
* **Encoding**: ASCII/UTF-8 assumed for text files.
* **Threading**: Single-threaded CLI; not thread-safe (by design).
* **Error Handling**: Input validation for numbers & dates; conservative fallbacks (e.g., default to today if parse fails).

---

## 🚀 Ideas & Extensions

* **Persistence**: switch to SQLite or JSON/CSV with headers.
* **Validation**: richer checks (phone/email formats).
* **Search**: more fields, pagination.
* **Import/Export**: CSV export for reports.
* **Unit Tests**: date helpers, service-level operations.
* **Internationalization**: date formats, currency display.

---

## 📄 License

MIT — feel free to use this as a learning or demo project.
*(Replace with your preferred license if needed.)*

---

## 🙌 Credit / Notes

* Written to be easy to read and present in interviews.
* Emphasizes *clarity* over frameworks or dependencies.
* If you split `main.cpp` later, consider `src/` + `include/` + CMake for larger projects.

---
