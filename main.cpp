#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <limits>
#include <cmath>

using namespace std;


//Utilities 
static inline string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static vector<string> split(const string &line, char delim='|') {
    vector<string> out; string cur;
    for (char c : line) {
        if (c == delim) { out.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    out.push_back(cur);
    return out;
}

static string join(const vector<string> &v, char delim='|') {
    string s;
    for (size_t i=0;i<v.size();++i) {
        if (i) s.push_back(delim);
        s += v[i];
    }
    return s;
}

static bool isNumber(const string &s) {
    if (s.empty()) return false;
    for (char c : s) if (!isdigit((unsigned char)c)) return false;
    return true;
}


// Date Helpers 
struct Date {
    int y=1970, m=1, d=1;
};

static bool isLeap(int y) {
    return (y%400==0) || (y%4==0 && y%100!=0);
}

static int daysInMonth(int y, int m) {
    static int md[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m==2) return isLeap(y)?29:28;
    return md[m-1];
}

static bool parseDate(const string &s, Date &out) {
    if (s.size()!=10 || s[4]!='-' || s[7]!='-') return false;
    string sy = s.substr(0,4), sm = s.substr(5,2), sd = s.substr(8,2);
    if (!isNumber(sy) || !isNumber(sm) || !isNumber(sd)) return false;
    int y = stoi(sy), m = stoi(sm), d = stoi(sd);
    if (m<1 || m>12) return false;
    if (d<1 || d>daysInMonth(y,m)) return false;
    out.y=y; out.m=m; out.d=d; return true;
}

static string dateToString(const Date &dt) {
    auto two = [](int v){ string s = to_string(v); if (s.size()==1) s="0"+s; return s; };
    return to_string(dt.y) + "-" + two(dt.m) + "-" + two(dt.d);
}

static Date addMonths(Date dt, int months) {
    int Y = dt.y;
    int M = dt.m + months;
    while (M > 12) { M -= 12; Y++; }
    while (M < 1)  { M += 12; Y--; }
    int D = min(dt.d, daysInMonth(Y, M));
    return Date{Y, M, D};
}

static int cmpDate(const Date &a, const Date &b) {
    if (a.y!=b.y) return a.y<b.y?-1:1;
    if (a.m!=b.m) return a.m<b.m?-1:1;
    if (a.d!=b.d) return a.d<b.d?-1:1;
    return 0;
}

static Date todayApprox() {
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    return Date{1900 + lt->tm_year, 1 + lt->tm_mon, lt->tm_mday};
}

//Entities
class Person {
protected:
    string name;
    string contact;
    string address;
public:
    Person() {}
    Person(const string &n, const string &c, const string &a) : name(n), contact(c), address(a) {}
    virtual ~Person() {}

    string getName() const { return name; }
    string getContact() const { return contact; }
    string getAddress() const { return address; }

    void setName(const string &n) { name = n; }
    void setContact(const string &c) { contact = c; }
    void setAddress(const string &a) { address = a; }
};

class Client : public Person {
    int id;
    int age;
public:
    Client() : id(0), age(0) {}
    Client(int i, const string &n, int ag, const string &c, const string &a)
        : Person(n,c,a), id(i), age(ag) {}

    int getId() const { return id; }
    int getAge() const { return age; }
    void setId(int i) { id = i; }
    void setAge(int ag) { age = ag; }

    static Client fromRecord(const string &line) {
        auto v = split(line, '|');
        // id|name|age|contact|address
        Client c;
        if (v.size()==5 && isNumber(v[0]) && isNumber(v[2])) {
            c.id = stoi(v[0]);
            c.name = v[1];
            c.age = stoi(v[2]);
            c.contact = v[3];
            c.address = v[4];
        }
        return c;
    }
    string toRecord() const {
        return join({to_string(id), name, to_string(age), contact, address}, '|');
    }
};

class Policy {
    string policyId;
    string type;
    double monthlyPremium;
    int durationMonths;
    int clientId;
    string startDate;
public:
    Policy() : monthlyPremium(0.0), durationMonths(0), clientId(0) {}

    string getPolicyId()     const { return policyId; }
    string getType()         const { return type; }
    double getPremium()      const { return monthlyPremium; }
    int    getDuration()     const { return durationMonths; }
    int    getClientId()     const { return clientId; }
    string getStartDate()    const { return startDate; }

    void setPolicyId(const string &id) { policyId = id; }
    void setType(const string &t) { type = t; }
    void setPremium(double p) { monthlyPremium = p; }
    void setDuration(int m) { durationMonths = m; }
    void setClientId(int cid) { clientId = cid; }
    void setStartDate(const string &sd) { startDate = sd; }

    static Policy fromRecord(const string &line) {
        // policyId|type|premium|duration|clientId|startDate
        auto v = split(line, '|');
        Policy p;
        if (v.size() >= 5) {
            p.policyId = v[0];
            p.type = v[1];
            p.monthlyPremium = (v.size() >= 3 ? atof(v[2].c_str()) : 0.0);
            p.durationMonths = (v.size() >= 4 && isNumber(v[3]) ? stoi(v[3]) : 0);
            p.clientId = (v.size() >= 5 && isNumber(v[4]) ? stoi(v[4]) : 0);
            if (v.size() >= 6) p.startDate = v[5];
            else p.startDate = dateToString(todayApprox());
        }
        return p;
    }
    string toRecord() const {
        return join({
            policyId, type, to_string((long double)monthlyPremium),
            to_string(durationMonths), to_string(clientId), startDate
        }, '|');
    }
};

class Payment {
    string policyId;
    double amount;
    string date; 
public:
    Payment(): amount(0.0) {}
    Payment(const string &pid, double amt, const string &dt)
        : policyId(pid), amount(amt), date(dt) {}

    string getPolicyId() const { return policyId; }
    double getAmount()   const { return amount; }
    string getDate()     const { return date;   }

    void setPolicyId(const string &pid) { policyId = pid; }
    void setAmount(double a) { amount = a; }
    void setDate(const string &d) { date = d; }

    static Payment fromRecord(const string &line) {
        // policyId|amount|date
        auto v = split(line, '|');
        Payment pm;
        if (v.size()==3) {
            pm.policyId = v[0];
            pm.amount = atof(v[1].c_str());
            pm.date = v[2];
        }
        return pm;
    }
    string toRecord() const {
        return join({policyId, to_string((long double)amount), date}, '|');
    }
};


//Services (Main functions for my app)
class ClientService {
    vector<Client> clients;
    string filename;
public:
    ClientService(const string &file="clients.txt") : filename(file) { load(); }

    void load() {
        clients.clear();
        ifstream in(filename);
        if (!in) return;
        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            clients.push_back(Client::fromRecord(line));
        }
    }
    
    void save() {
        ofstream out(filename);
        for (auto &c : clients) out << c.toRecord() << "\n";
    }

    int nextId() const {
        int mx = 1000;
        for (auto &c : clients) mx = max(mx, c.getId());
        return mx + 1;
    }

    bool addClient(const string &name, int age, const string &contact, const string &addr, int &outId) {
        Client c(nextId(), name, age, contact, addr);
        clients.push_back(c);
        save();
        outId = c.getId();
        return true;
    }

    Client* findById(int id) {
        for (auto &c : clients) if (c.getId()==id) return &c;
        return nullptr;
    }

    vector<Client*> findByName(const string &kw) {
        vector<Client*> out;
        string needle = kw; transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
        for (auto &c : clients) {
            string n = c.getName();
            transform(n.begin(), n.end(), n.begin(), ::tolower);
            if (n.find(needle) != string::npos) out.push_back(&c);
        }
        return out;
    }

    bool updateClient(int id, const string &name, const string &ageStr,const string &contact, const string &addr) {
        Client *c = findById(id);
        if (!c) return false;
        if (!name.empty()) c->setName(name);
        if (!ageStr.empty() && isNumber(ageStr)) c->setAge(stoi(ageStr));
        if (!contact.empty()) c->setContact(contact);
        if (!addr.empty()) c->setAddress(addr);
        save();
        return true;
    }

    bool removeClient(int id, bool hasPolicies) {
        if (hasPolicies) return false;
        auto it = remove_if(clients.begin(), clients.end(),
                            [&](const Client &c){ return c.getId()==id; });
        if (it==clients.end()) return false;
        clients.erase(it, clients.end());
        save();
        return true;
    }

    const vector<Client>& getAll() const { return clients; }
};

class PolicyService {
    vector<Policy> policies;
    string filename;
public:
    PolicyService(const string &file="policies.txt") : filename(file) { load(); }

    void load() {
        policies.clear();
        ifstream in(filename);
        if (!in) return;
        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            policies.push_back(Policy::fromRecord(line));
        }
    }
    
    void save() {
        ofstream out(filename);
        for (auto &p : policies) out << p.toRecord() << "\n";
    }

    string nextPolicyId() const {
        int mx = 1000;
        for (auto &p : policies) {
            const string &pid = p.getPolicyId();
            if (!pid.empty() && (pid[0]=='P' || pid[0]=='p')) {
                string num = pid.substr(1);
                if (isNumber(num)) mx = max(mx, stoi(num));
            }
        }
        return string("P") + to_string(mx + 1);
    }

    bool addPolicy(int clientId, const string &type, double premium, int months, const string &start, string &outPid) {
        Policy p;
        p.setClientId(clientId);
        p.setType(type);
        p.setPremium(premium);
        p.setDuration(months);
        Date dt;
        if (!parseDate(start, dt)) {
            p.setStartDate(dateToString(todayApprox()));
        } else {
            p.setStartDate(start);
        }
        p.setPolicyId(nextPolicyId());
        policies.push_back(p);
        save();
        outPid = p.getPolicyId();
        return true;
    }

    Policy* findByPolicyId(const string &pid) {
        for (auto &p : policies) if (p.getPolicyId()==pid) return &p;
        return nullptr;
    }
    
    vector<Policy*> findByClientId(int cid) {
        vector<Policy*> out;
        for (auto &p : policies) if (p.getClientId()==cid) out.push_back(&p);
        return out;
    }

    bool updatePolicy(const string &pid, const string &type, const string &prem,const string &months, const string &start) {
        Policy *p = findByPolicyId(pid);
        if (!p) return false;
        if (!type.empty()) p->setType(type);
        if (!prem.empty()) p->setPremium(atof(prem.c_str()));
        if (!months.empty() && isNumber(months)) p->setDuration(stoi(months));
        if (!start.empty()) {
            Date dt;
            if (parseDate(start, dt)) p->setStartDate(start);
        }
        save();
        return true;
    }

    bool removePolicy(const string &pid, bool hasPayments) {
        if (hasPayments) return false;
        auto it = remove_if(policies.begin(), policies.end(),
                            [&](const Policy &p){ return p.getPolicyId()==pid; });
        if (it==policies.end()) return false;
        policies.erase(it, policies.end());
        save();
        return true;
    }

    // Business helpers
    static bool policyEndDate(const Policy &p, Date &endDate) {
        Date st;
        if (!parseDate(p.getStartDate(), st)) return false;
        endDate = addMonths(st, p.getDuration());
        return true;
    }

    const vector<Policy>& getAll() const { return policies; }
};

class PaymentService {
    vector<Payment> payments;
    string filename;
public:
    PaymentService(const string &file="payments.txt") : filename(file) { load(); }

    void load() {
        payments.clear();
        ifstream in(filename);
        if (!in) return;
        string line;
        while (getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            payments.push_back(Payment::fromRecord(line));
        }
    }
    
    void save() {
        ofstream out(filename);
        for (auto &pm : payments) out << pm.toRecord() << "\n";
    }

    void recordPayment(const string &pid, double amount, const string &dateStr) {
        Date dt;
        string d = dateStr;
        if (!parseDate(d, dt)) d = dateToString(todayApprox());
        payments.emplace_back(pid, amount, d);
        save();
    }

    vector<Payment> findByPolicyId(const string &pid) const {
        vector<Payment> out;
        for (auto &pm : payments) if (pm.getPolicyId()==pid) out.push_back(pm);
        sort(out.begin(), out.end(), [](const Payment &a, const Payment &b){
            Date da, db;
            bool pa = parseDate(a.getDate(), da);
            bool pb = parseDate(b.getDate(), db);
            if (pa && pb) return cmpDate(da, db) < 0;
            return a.getDate() < b.getDate();
        });
        return out;
    }

    double totalPaid(const string &pid) const {
        double s=0.0;
        for (auto &pm : payments) if (pm.getPolicyId()==pid) s += pm.getAmount();
        return s;
    }

    bool hasPayments(const string &pid) const {
        for (auto &pm : payments) if (pm.getPolicyId()==pid) return true;
        return false;
    }

    void deletePaymentsOf(const string &pid) {
        auto it = remove_if(payments.begin(), payments.end(),
                            [&](const Payment &pm){ return pm.getPolicyId()==pid; });
        if (it != payments.end()) {
            payments.erase(it, payments.end());
            save();
        }
    }

    const vector<Payment>& getAll() const { return payments; }
};

// Business login for my reference
static int approxMonthsPaid(double monthlyPremium, double totalPaid) {
    if (monthlyPremium <= 0) return 0;
    return (int)floor(totalPaid / monthlyPremium);
}

static bool nextDueDate(const Policy &p, const PaymentService &paySvc, Date &due) {
    Date st;
    if (!parseDate(p.getStartDate(), st)) return false;
    double paid = paySvc.totalPaid(p.getPolicyId());
    int monthsPaid = approxMonthsPaid(p.getPremium(), paid);
    if (monthsPaid >= p.getDuration()) return false; // finished
    due = addMonths(st, monthsPaid + 1);
    return true;
}

static double remainingBalance(const Policy &p, const PaymentService &paySvc) {
    double total = p.getPremium() * p.getDuration();
    double paid  = paySvc.totalPaid(p.getPolicyId());
    return max(0.0, total - paid);
}

//Reports Step 4 ,polymorphism
class Report {
public:
    virtual ~Report() {}
    virtual void generate() = 0;  // polymorphic interface
};

class AllClientsReport : public Report {
    const ClientService &cs;
public:
    AllClientsReport(const ClientService &c) : cs(c) {}
    void generate() override {
        cout << left << setw(8) << "ID" << setw(22) << "Name" << setw(6) << "Age"
             << setw(15) << "Contact" << "Address\n";
        for (auto &c : cs.getAll()) {
            cout << left << setw(8) << c.getId() << setw(22) << c.getName()
                 << setw(6) << c.getAge() << setw(15) << c.getContact()
                 << c.getAddress() << "\n";
        }
    }
};

class AllPoliciesReport : public Report {
    const PolicyService &ps;
public:
    AllPoliciesReport(const PolicyService &p) : ps(p) {}
    void generate() override {
        cout << left << setw(10) << "PolicyID" << setw(8) << "Client" << setw(12) << "Type"
             << setw(12) << "Premium" << setw(10) << "Months" << setw(12) << "Start" << "\n";
        for (auto &p : ps.getAll()) {
            cout << left << setw(10) << p.getPolicyId() << setw(8) << p.getClientId()
                 << setw(12) << p.getType() << setw(12) << (long double)p.getPremium()
                 << setw(10) << p.getDuration() << setw(12) << p.getStartDate() << "\n";
        }
    }
};

class ExpiringPoliciesReport : public Report {
    const PolicyService &ps;
    const ClientService &cs;
    int N;
public:
    ExpiringPoliciesReport(const PolicyService &p, const ClientService &c, int n)
        : ps(p), cs(c), N(n) {}
    void generate() override {
        Date now = todayApprox();
        Date endWindow = addMonths(now, N);
        cout << "Window End: " << dateToString(endWindow) << "\n";
        cout << left << setw(10) << "PolicyID" << setw(8) << "Client" << setw(22) << "ClientName"
             << setw(12) << "EndDate" << "\n";
        for (auto &p : ps.getAll()) {
            Date ed;
            if (!PolicyService::policyEndDate(p, ed)) continue;
            if (cmpDate(now, ed) <= 0 && cmpDate(ed, endWindow) <= 0) {
                const Client *cptr = nullptr;
                for (auto &c : cs.getAll()) if (c.getId()==p.getClientId()) { cptr = &c; break; }
                string cname = cptr ? cptr->getName() : "[Unknown]";
                cout << left << setw(10) << p.getPolicyId() << setw(8) << p.getClientId()
                     << setw(22) << cname << setw(12) << dateToString(ed) << "\n";
            }
        }
    }
};

class UnpaidClientsReport : public Report {
    const PolicyService &ps;
    const ClientService &cs;
    const PaymentService &pay;
public:
    UnpaidClientsReport(const PolicyService &p, const ClientService &c, const PaymentService &pm)
        : ps(p), cs(c), pay(pm) {}
    void generate() override {
        cout << left << setw(8) << "Client" << setw(22) << "Name"
             << setw(12) << "PolicyID" << setw(12) << "Remaining" << "\n";
        for (auto &p : ps.getAll()) {
            double rem = remainingBalance(p, pay);
            if (rem > 1e-9) {
                const Client *cptr = nullptr;
                for (auto &c : cs.getAll()) if (c.getId()==p.getClientId()) { cptr = &c; break; }
                string cname = cptr ? cptr->getName() : "[Unknown]";
                cout << left << setw(8) << p.getClientId() << setw(22) << cname
                     << setw(12) << p.getPolicyId() << setw(12) << (long double)rem << "\n";
            }
        }
    }
};


//My main menu displayed
class Application {
    ClientService clientSvc;
    PolicyService policySvc;
    PaymentService paymentSvc;

public:
    Application() : clientSvc("clients.txt"), policySvc("policies.txt"), paymentSvc("payments.txt") {}

    //Client Management
    void addClient() {
        string name, contact, address;
        int age;
        cout << "Name: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, name);
        cout << "Age: ";
        cin >> age;
        cout << "Contact: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, contact);
        cout << "Address: ";
        getline(cin, address);

        int id=0;
        if (clientSvc.addClient(name, age, contact, address, id))
            cout << "[OK] Client added with ID: " << id << "\n";
        else
            cout << "[ERR] Unable to add client.\n";
    }

    void viewClient() {
        cout << "Enter Client ID: ";
        int id; cin >> id;
        Client* c = clientSvc.findById(id);
        if (!c) { cout << "[ERR] Client not found.\n"; return; }
        cout << "== Client ==\n";
        cout << "ID: " << c->getId() << "\nName: " << c->getName() << "\nAge: " << c->getAge()
             << "\nContact: " << c->getContact() << "\nAddress: " << c->getAddress() << "\n";
    }

    void searchClient() {
        cout << "Search by 1) ID  2) Name : ";
        int ch; cin >> ch;
        if (ch == 1) {
            cout << "Enter ID: "; int id; cin >> id;
            Client* c = clientSvc.findById(id);
            if (!c) { cout << "[ERR] Client not found.\n"; return; }
            cout << c->getId() << " | " << c->getName() << " | Age " << c->getAge()
                 << " | " << c->getContact() << " | " << c->getAddress() << "\n";
        } else {
            cout << "Enter name keyword: ";
            string s; cin.ignore(numeric_limits<streamsize>::max(), '\n'); getline(cin, s);
            auto res = clientSvc.findByName(s);
            if (res.empty()) { cout << "[INFO] No matches.\n"; return; }
            for (auto *c : res) {
                cout << c->getId() << " | " << c->getName() << " | Age " << c->getAge()
                     << " | " << c->getContact() << " | " << c->getAddress() << "\n";
            }
        }
    }

    void updateClient() {
        cout << "Enter Client ID to update: ";
        int id; cin >> id;
        Client* c = clientSvc.findById(id);
        if (!c) { cout << "[ERR] Client not found.\n"; return; }
        cout << "Leave empty to keep existing. Press ENTER after prompts.\n";
        cout << "Name (" << c->getName() << "): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string name; getline(cin, name);
        cout << "Age (" << c->getAge() << "): ";
        string age; getline(cin, age);
        cout << "Contact (" << c->getContact() << "): ";
        string contact; getline(cin, contact);
        cout << "Address (" << c->getAddress() << "): ";
        string address; getline(cin, address);

        if (clientSvc.updateClient(id, name, age, contact, address))
            cout << "[OK] Client updated.\n";
        else
            cout << "[ERR] Update failed.\n";
    }

    void deleteClient() {
        cout << "Enter Client ID to delete: ";
        int id; cin >> id;
        bool hasPolicies = !policySvc.findByClientId(id).empty();
        if (!clientSvc.removeClient(id, hasPolicies))
            cout << "[ERR] Cannot delete: either not found or client has policies.\n";
        else
            cout << "[OK] Client deleted.\n";
    }

    void clientMenu() {
        while (true) {
            cout << "\n== Client Management ==\n"
                 << "1) Add Client\n2) View Client\n3) Search Client\n4) Update Client\n5) Delete Client\n0) Back\n> ";
            int ch; cin >> ch;
            switch (ch) {
                case 1: addClient(); break;
                case 2: viewClient(); break;
                case 3: searchClient(); break;
                case 4: updateClient(); break;
                case 5: deleteClient(); break;
                case 0: return;
                default: cout << "Invalid choice.\n"; break;
            }
        }
    }

    // Policy Management 
    void addPolicy() {
        int clientId; string type, start;
        double premium; int months;

        cout << "Client ID: ";
        cin >> clientId;
        if (!clientSvc.findById(clientId)) { cout << "[ERR] Client not found.\n"; return; }

        cout << "Policy Type (e.g., Life/Health/Auto): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, type);

        cout << "Monthly Premium: ";
        cin >> premium;

        cout << "Duration (months): ";
        cin >> months;

        cout << "Policy Start Date (YYYY-MM-DD): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, start);

        string pid;
        if (policySvc.addPolicy(clientId, type, premium, months, start, pid))
            cout << "[OK] Policy created: " << pid << "\n";
        else
            cout << "[ERR] Failed to create policy.\n";
    }

    void viewAllPolicies() {
        AllPoliciesReport rpt(policySvc);
        rpt.generate();
    }

    void searchPolicy() {
        cout << "Search by 1) PolicyID  2) ClientID : ";
        int ch; cin >> ch;
        if (ch == 1) {
            cout << "Enter Policy ID (e.g., P1001): ";
            string pid; cin >> pid;
            Policy* p = policySvc.findByPolicyId(pid);
            if (!p) { cout << "[ERR] Policy not found.\n"; return; }
            cout << p->getPolicyId() << " | " << p->getType() << " | Premium " << p->getPremium()
                 << " | Months " << p->getDuration() << " | Client " << p->getClientId()
                 << " | Start " << p->getStartDate() << "\n";
        } else {
            cout << "Enter Client ID: ";
            int cid; cin >> cid;
            auto v = policySvc.findByClientId(cid);
            if (v.empty()) { cout << "[INFO] No policies for client.\n"; return; }
            for (auto *p : v) {
                cout << p->getPolicyId() << " | " << p->getType() << " | Premium " << p->getPremium()
                     << " | Months " << p->getDuration() << " | Start " << p->getStartDate() << "\n";
            }
        }
    }

    void updatePolicy() {
        cout << "Enter Policy ID to update: ";
        string pid; cin >> pid;
        Policy* p = policySvc.findByPolicyId(pid);
        if (!p) { cout << "[ERR] Policy not found.\n"; return; }

        cout << "Leave empty to keep existing (type/premium/months/startDate).\n";
        cout << "Type (" << p->getType() << "): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string type; getline(cin, type);
        cout << "Monthly Premium (" << p->getPremium() << "): ";
        string prem; getline(cin, prem);
        cout << "Duration Months (" << p->getDuration() << "): ";
        string months; getline(cin, months);
        cout << "Start Date (" << p->getStartDate() << "): ";
        string start; getline(cin, start);

        if (policySvc.updatePolicy(pid, type, prem, months, start))
            cout << "[OK] Policy updated.\n";
        else
            cout << "[ERR] Update failed.\n";
    }

    void deletePolicy() {
        cout << "Enter Policy ID to delete: ";
        string pid; cin >> pid;
        bool hasPmts = paymentSvc.hasPayments(pid);
        if (!policySvc.removePolicy(pid, hasPmts))
            cout << "[ERR] Cannot delete policy (payments exist or not found).\n";
        else
            cout << "[OK] Policy deleted.\n";
    }

    void policyMenu() {
        while (true) {
            cout << "\n== Policy Management ==\n"
                 << "1) Add Policy\n2) View All Policies\n3) Search Policy\n4) Update Policy\n5) Delete Policy\n0) Back\n> ";
            int ch; cin >> ch;
            switch (ch) {
                case 1: addPolicy(); break;
                case 2: viewAllPolicies(); break;
                case 3: searchPolicy(); break;
                case 4: updatePolicy(); break;
                case 5: deletePolicy(); break;
                case 0: return;
                default: cout << "Invalid choice.\n"; break;
            }
        }
    }

    // Payments & Status
    void recordPayment() {
        cout << "Policy ID: ";
        string pid; cin >> pid;
        Policy* p = policySvc.findByPolicyId(pid);
        if (!p) { cout << "[ERR] Policy not found.\n"; return; }

        double amount; string dateStr;
        cout << "Amount: ";
        cin >> amount;
        cout << "Payment Date (YYYY-MM-DD): ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, dateStr);

        paymentSvc.recordPayment(pid, amount, dateStr);
        cout << "[OK] Payment recorded.\n";
    }

    void showPaymentHistory() {
        cout << "Policy ID: ";
        string pid; cin >> pid;
        Policy* p = policySvc.findByPolicyId(pid);
        if (!p) { cout << "[ERR] Policy not found.\n"; return; }

        auto v = paymentSvc.findByPolicyId(pid);
        if (v.empty()) { cout << "[INFO] No payments.\n"; return; }
        cout << "Date        | Amount\n";
        for (auto &pm : v) {
            cout << setw(12) << pm.getDate() << " | " << (long double)pm.getAmount() << "\n";
        }
    }

    void calcNextDueOrRemaining() {
        cout << "Policy ID: ";
        string pid; cin >> pid;
        Policy* p = policySvc.findByPolicyId(pid);
        if (!p) { cout << "[ERR] Policy not found.\n"; return; }

        cout << "== Balance & Due ==\n";
        cout << "Monthly Premium: " << p->getPremium() << "\n";
        cout << "Duration: " << p->getDuration() << " months\n";
        cout << "Start: " << p->getStartDate() << "\n";

        double total = p->getPremium() * p->getDuration();
        double paid  = paymentSvc.totalPaid(p->getPolicyId());
        cout << "Total Due (full term): " << (long double)total << "\n";
        cout << "Total Paid: " << (long double)paid << "\n";
        int mp = approxMonthsPaid(p->getPremium(), paid);
        cout << "Months Paid (approx): " << mp << " / " << p->getDuration() << "\n";
        Date due;
        if (nextDueDate(*p, paymentSvc, due)) {
            cout << "Next Due Date: " << dateToString(due) << "\n";
        } else {
            cout << "No further dues (fully paid or invalid start date).\n";
        }
        cout << "Remaining Balance: " << (long double)remainingBalance(*p, paymentSvc) << "\n";
    }

    void policyStatusReport() {
        cout << "Policy ID: ";
        string pid; cin >> pid;
        Policy* p = policySvc.findByPolicyId(pid);
        if (!p) { cout << "[ERR] Policy not found.\n"; return; }
        Client* c = clientSvc.findById(p->getClientId());
        cout << "== Policy Status ==\n";
        if (c) cout << "Client: " << c->getId() << " - " << c->getName() << "\n";
        else   cout << "Client: " << p->getClientId() << " - [Unknown]\n";
        cout << "Type: " << p->getType() << " | Premium: " << (long double)p->getPremium()
             << " | Duration: " << p->getDuration() << " | Start: " << p->getStartDate() << "\n";
        double total = p->getPremium() * p->getDuration();
        double paid  = paymentSvc.totalPaid(p->getPolicyId());
        cout << "Total Due (full term): " << (long double)total << "\n";
        cout << "Total Paid: " << (long double)paid << "\n";
        cout << "Months Paid (approx): " << approxMonthsPaid(p->getPremium(), paid)
             << " / " << p->getDuration() << "\n";
        Date due;
        if (nextDueDate(*p, paymentSvc, due)) cout << "Next Due Date: " << dateToString(due) << "\n";
        else cout << "Next Due Date: N/A (complete or invalid)\n";
        cout << "Remaining Balance: " << (long double)remainingBalance(*p, paymentSvc) << "\n";
    }

    void paymentsMenu() {
        while (true) {
            cout << "\n== Premium Payments / Status ==\n"
                 << "1) Record Payment\n2) Show Payment History\n3) Next Due / Remaining Balance\n4) Policy Status Report\n0) Back\n> ";
            int ch; cin >> ch;
            switch (ch) {
                case 1: recordPayment(); break;
                case 2: showPaymentHistory(); break;
                case 3: calcNextDueOrRemaining(); break;
                case 4: policyStatusReport(); break;
                case 0: return;
                default: cout << "Invalid choice.\n"; break;
            }
        }
    }

    // Reports
    void reportsMenu() {
        while (true) {
            cout << "\n== Reports ==\n"
                 << "1) List All Clients\n2) List All Policies\n3) Policies Expiring in Next N Months\n4) Clients with Unpaid Premiums\n0) Back\n> ";
            int ch; cin >> ch;
            switch (ch) {
                case 1: {
                    AllClientsReport rpt(clientSvc);
                    Report &r = rpt;      // polymorphic call
                    r.generate();
                } break;
                case 2: {
                    AllPoliciesReport rpt(policySvc);
                    Report &r = rpt;
                    r.generate();
                } break;
                case 3: {
                    cout << "Enter N (months): ";
                    int N; cin >> N;
                    ExpiringPoliciesReport rpt(policySvc, clientSvc, N);
                    Report &r = rpt;
                    r.generate();
                } break;
                case 4: {
                    UnpaidClientsReport rpt(policySvc, clientSvc, paymentSvc);
                    Report &r = rpt;
                    r.generate();
                } break;
                case 0: return;
                default: cout << "Invalid choice.\n"; break;
            }
        }
    }

    // MAIN MENU
    void run() {
        while (true) {
            cout << "\n==============================\n";
            cout << "Insurance Policy Management\n";
            cout << "==============================\n";
            cout << "1) Client Management\n";
            cout << "2) Policy Management\n";
            cout << "3) Premium Payments / Status\n";
            cout << "4) Reports\n";
            cout << "0) Exit\n> "<< flush;
            int ch;
            if (!(cin >> ch)) break;

            switch (ch) {
                case 1: clientMenu(); break;
                case 2: policyMenu(); break;
                case 3: paymentsMenu(); break;
                case 4: reportsMenu(); break;
                case 0:
                    cout << "Goodbye!\n"; 
                    return;
                default:
                    cout << "Invalid choice.\n";
                    break;
            }
        }
    }
};

int main() {
    cin.tie(&cout);
    Application app;
    app.run();
    return 0;
}
