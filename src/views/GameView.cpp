#include "GameView.hpp"
#include "ColorGroup.hpp"
#include "StreetTile.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#include <cstdlib>
#endif

using namespace std;

GameView::GameView() {}

GameView::~GameView() {}

void GameView::clearScreen() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void GameView::displayMainMenu() const {
    cout << "============================================\n";
    cout << "           N I M O N S P O L I             \n";
    cout << "============================================\n";
    cout << "  1. Mulai Permainan Baru\n";
    cout << "  2. Muat Permainan Tersimpan\n";
    cout << "  3. Keluar\n";
    cout << "============================================\n";
    cout << "Pilih (1/2/3): ";
}

vector<string> GameView::promptPlayerSetup() const {
    int count = 0;
    while (true) {
        cout << "Jumlah pemain (2-4): ";
        if (cin >> count && count >= 2 && count <= 4) {
            break;
        }
        cout << "Input tidak valid. Jumlah pemain harus antara 2 hingga 4.\n";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    vector<string> names(count);
    for (int i = 0; i < count; ++i) {
        while (true) {
            cout << "Nama pemain " << (i + 1) << ": ";
            cin >> names[i];

            bool duplicate = false;
            for (int j = 0; j < i; ++j) {
                if (names[j] == names[i]) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) {
                cout << "Nama " << names[i] << " sudah digunakan. Masukkan nama lain.\n";
            } else {
                break;
            }
        }
    }
    cin.ignore();
    return names;
}

void GameView::printBoard(const Board& board, const vector<unique_ptr<Player>>& players,
                          int currentTurn, int maxTurn) const {
    int totalTiles = board.getTotalTiles();
    if (totalTiles == 0) {
        cout << "[Board kosong]\n";
        return;
    }

    const int CW = 10; // cell content width (visible chars)
    const int COLS = 11;
    const int INNER_W = 9 * (CW + 1) - 1; // 98 — interior span between border cells

    // ── ANSI colour helpers ─────────────────────────────────────────────────────
    auto ansi = [](const string& cc) -> string {
        if (cc == "CK") {
            return "\033[33m"; // brown → yellow
        }
        if (cc == "BM") {
            return "\033[96m"; // light blue → bright cyan
        }
        if (cc == "PK") {
            return "\033[95m"; // pink → bright magenta
        }
        if (cc == "OR") {
            return "\033[38;5;208m"; // orange
        }
        if (cc == "MR") {
            return "\033[91m"; // red
        }
        if (cc == "KN") {
            return "\033[93m"; // yellow
        }
        if (cc == "HJ") {
            return "\033[92m"; // green
        }
        if (cc == "BT") {
            return "\033[94m"; // dark blue
        }
        if (cc == "AB") {
            return "\033[97m"; // utility → bright white
        }
        return "\033[37m"; // DF → white
    };
    const string RST = "\033[0m";

    // ── Helpers ─────────────────────────────────────────────────────────────────
    auto pad = [](const string& s, int w) -> string {
        if (static_cast<int>(s.size()) >= w)
            return s.substr(0, w);
        return s + string(w - static_cast<int>(s.size()), ' ');
    };

    auto centerStr = [](const string& s, int w) -> string {
        if (static_cast<int>(s.size()) >= w)
            return s.substr(0, w);
        int lp = (w - static_cast<int>(s.size())) / 2;
        string r = string(lp, ' ') + s;
        r += string(w - static_cast<int>(r.size()), ' ');
        return r;
    };

    // ── Position mapping: grid (row,col) → board position ───────────────────────
    auto getPos = [](int row, int col) -> int {
        if (row == 10)
            return 10 - col; // bottom row: right-to-left
        if (row == 0)
            return 20 + col; // top row:    left-to-right
        if (col == 0)
            return 20 - row;    // left col:   top-to-bottom
        return (30 + row) % 40; // right col:  top-to-bottom
    };

    // ── Colour-group code for tile ──────────────────────────────────────────────
    auto getCC = [](Tile* t) -> string {
        if (auto* s = dynamic_cast<const StreetTile*>(t)) {
            switch (s->getColorGroup()) {
            case ColorGroup::BROWN:
                return "CK";
            case ColorGroup::LIGHT_BLUE:
                return "BM";
            case ColorGroup::PINK:
                return "PK";
            case ColorGroup::ORANGE:
                return "OR";
            case ColorGroup::RED:
                return "MR";
            case ColorGroup::YELLOW:
                return "KN";
            case ColorGroup::GREEN:
                return "HJ";
            case ColorGroup::DARK_BLUE:
                return "BT";
            }
        }
        if (auto* p = dynamic_cast<const PropertyTile*>(t))
            if (p->getType() == PropertyType::UTILITY)
                return "AB";
        return "DF";
    };

    int jailPos = board.getJailPosition();

    // ── Print cell Line 1: [CC] NNN  (ANSI coloured bracket) ────────────────────
    auto printL1 = [&](int pos) {
        Tile* t = board.getTileAt(pos);
        if (!t) {
            cout << string(CW, ' ');
            return;
        }
        string cc = getCC(t);
        string code = t->getCode();
        while (static_cast<int>(code.size()) < 3)
            code += ' ';
        code = code.substr(0, 3);
        // visible width = [CC] NNN = 4+1+3 = 8, pad remaining
        cout << ansi(cc) << "[" << cc << "]" << RST << " " << code << string(max(0, CW - 8), ' ');
    };

    // ── Print cell Line 2: ownership / building / player tokens ─────────────────
    auto printL2 = [&](int pos) {
        Tile* t = board.getTileAt(pos);
        if (!t) {
            cout << string(CW, ' ');
            return;
        }
        string content;

        if (pos == jailPos) {
            // Jail: show IN:<player-ids> V:<player-ids>
            string inIds, visIds;
            for (const auto& p : players) {
                if (p->getStatus() == PlayerStatus::BANKRUPT)
                    continue;
                if (p->getPosition() != jailPos)
                    continue;
                if (p->isInJail())
                    inIds += to_string(p->getId() + 1);
                else
                    visIds += to_string(p->getId() + 1);
            }
            if (!inIds.empty())
                content += "IN:" + inIds;
            if (!visIds.empty()) {
                if (!content.empty())
                    content += " ";
                content += "V:" + visIds;
            }
        } else {
            // Owner + building level
            auto* prop = dynamic_cast<PropertyTile*>(t);
            if (prop && prop->getOwner()) {
                content += "P" + to_string(prop->getOwner()->getId() + 1);
                if (auto* st = dynamic_cast<const StreetTile*>(prop)) {
                    int lvl = st->getPropertyLevel();
                    if (lvl == 5)
                        content += " *";
                    else if (lvl > 0)
                        content += " " + string(lvl, '^');
                }
            }
            // Player bidak markers
            for (const auto& p : players) {
                if (p->getStatus() == PlayerStatus::BANKRUPT)
                    continue;
                if (p->getPosition() != pos)
                    continue;
                string mk = "(" + to_string(p->getId() + 1) + ")";
                if (!content.empty() &&
                    static_cast<int>(content.size()) + static_cast<int>(mk.size()) + 1 <= CW)
                    content += " ";
                content += mk;
            }
        }
        cout << pad(content, CW);
    };

    // ── Separator lines ─────────────────────────────────────────────────────────
    auto printFullSep = [&]() {
        cout << "+";
        for (int c = 0; c < COLS; ++c)
            cout << string(CW, '-') << "+";
        cout << "\n";
    };

    // ── Interior content ────────────────────────────────────────────────────────
    // 9 interior rows × 2 content lines = 18 content slots
    // 8 partial-separator lines between interior rows = 8 separator slots
    // Total: 26 interior text positions
    vector<string> interior(26, pad("", INNER_W));

    string titleBox = string(34, '=');
    string titleTxt = "||          NIMONSPOLI          ||";
    string divider = string(34, '-');
    string turnStr =
        currentTurn >= 0 ? "TURN " + to_string(currentTurn) + " / " + to_string(maxTurn) : "";

    // Map: interior index layout
    // Row 1: content[0], content[1]     → L1, L2
    //         separator[0]               → sep between row 1-2
    // Row 2: content[2], content[3]     → L1, L2
    //         separator[1]               → sep between row 2-3
    // ...etc

    // Content lines (indices 0..17 → 9 rows × 2 lines)
    // c[0],c[1] = Row1 (SBY/BDG): empty
    interior[0] = pad("", INNER_W);
    interior[1] = pad("", INNER_W);
    // c[2],c[3] = Row2 (SMG/DEN): NIMONSPOLI box top + title
    interior[2] = centerStr(titleBox, INNER_W);
    interior[3] = centerStr(titleTxt, INNER_W);
    // c[4],c[5] = Row3 (DNU/FES): empty + TURN
    interior[4] = pad("", INNER_W);
    interior[5] = centerStr(turnStr, INNER_W);
    // c[6],c[7] = Row4 (MAL/MTR): empty + separator
    interior[6] = pad("", INNER_W);
    interior[7] = centerStr(divider, INNER_W);
    // c[8],c[9] = Row5 (STB/GUB): P1-P4 + ^
    interior[8] = centerStr("P1-P4 : Properti milik Pemain 1-4", INNER_W);
    interior[9] = centerStr("^     : Rumah Level 1", INNER_W);
    // c[10],c[11] = Row6 (YOG/KSP): ^^^ + *
    interior[10] = centerStr("^^^   : Rumah Level 3", INNER_W);
    interior[11] = centerStr("* : Hotel (Maksimal)", INNER_W);
    // c[12],c[13] = Row7 (SOL/JKT): separator + KODE WARNA
    interior[12] = centerStr(divider, INNER_W);
    interior[13] = centerStr("KODE WARNA:", INNER_W);
    // c[14],c[15] = Row8 (PLN/PBM): [BM]+[KN] + [PK]+[HJ]
    interior[14] = centerStr("[BM]=Biru Muda [KN]=Kuning", INNER_W);
    interior[15] = centerStr("[PK]=Pink      [HJ]=Hijau", INNER_W);
    // c[16],c[17] = Row9 (MGL/IKN): [DF]+[AB] + empty
    interior[16] = centerStr("[DF]=Aksi      [AB]=Utility", INNER_W);
    interior[17] = pad("", INNER_W);

    // Separator lines between interior rows (indices 18..25 → 8 separators)
    // sep[18] = between row1-2: empty
    interior[18] = pad("", INNER_W);
    // sep[19] = between row2-3: NIMONSPOLI box bottom
    interior[19] = centerStr(titleBox, INNER_W);
    // sep[20] = between row3-4: empty
    interior[20] = pad("", INNER_W);
    // sep[21] = between row4-5: LEGENDA heading
    interior[21] = centerStr("LEGENDA KEPEMILIKAN & STATUS", INNER_W);
    // sep[22] = between row5-6: ^^
    interior[22] = centerStr("^^    : Rumah Level 2", INNER_W);
    // sep[23] = between row6-7: (1)-(4)
    interior[23] = centerStr("(1)-(4): Bidak (IN=Tahanan, V=Mampir)", INNER_W);
    // sep[24] = between row7-8: [CK]+[MR]
    interior[24] = centerStr("[CK]=Coklat    [MR]=Merah", INNER_W);
    // sep[25] = between row8-9: [OR]+[BT]
    interior[25] = centerStr("[OR]=Orange    [BT]=Biru Tua", INNER_W);

    // ── Render board ────────────────────────────────────────────────────────────
    for (int row = 0; row < COLS; ++row) {
        // Separator before this row
        if (row == 0 || row == 1 || row == 10) {
            printFullSep();
        } else {
            // Partial separator with interior text
            int sepIdx = 18 + (row - 2); // row2→18, row3→19, ..., row9→25
            cout << "+" << string(CW, '-') << "+" << interior[sepIdx] << "+" << string(CW, '-')
                 << "+\n";
        }

        // --- Line 1 (tile name) ---
        cout << "|";
        if (row == 0 || row == 10) {
            for (int col = 0; col < COLS; ++col) {
                printL1(getPos(row, col));
                cout << "|";
            }
        } else {
            int cIdx = (row - 1) * 2;
            printL1(getPos(row, 0));
            cout << "|" << interior[cIdx] << "|";
            printL1(getPos(row, 10));
            cout << "|";
        }
        cout << "\n";

        // --- Line 2 (status) ---
        cout << "|";
        if (row == 0 || row == 10) {
            for (int col = 0; col < COLS; ++col) {
                printL2(getPos(row, col));
                cout << "|";
            }
        } else {
            int cIdx = (row - 1) * 2 + 1;
            printL2(getPos(row, 0));
            cout << "|" << interior[cIdx] << "|";
            printL2(getPos(row, 10));
            cout << "|";
        }
        cout << "\n";
    }
    printFullSep();

    // ── Player info ─────────────────────────────────────────────────────────────
    cout << "\nPEMAIN: ";
    for (const auto& p : players) {
        if (p->getStatus() != PlayerStatus::BANKRUPT) {
            cout << "P" << (p->getId() + 1) << "=" << p->getUsername() << "(M" << p->getMoney()
                 << ",pos" << p->getPosition() << ")  ";
        }
    }
    cout << "\n\n";
}

void GameView::printPropertyDeed(const PropertyTile& property) const {
    cout << "========================================\n";
    cout << "AKTA: " << property.getName() << " [" << property.getCode() << "]\n";
    cout << "Tipe: ";
    switch (property.getType()) {
    case PropertyType::STREET:
        cout << "Lahan";
        break;
    case PropertyType::RAILROAD:
        cout << "Stasiun";
        break;
    case PropertyType::UTILITY:
        cout << "Utility";
        break;
    }
    cout << "\n";
    cout << "Harga beli  : M" << property.getPrice() << "\n";
    cout << "Nilai gadai : M" << property.getMortgageValue() << "\n";
    cout << "Status      : ";
    if (property.isBankOwned())
        cout << "Milik Bank";
    else if (property.isMortgaged())
        cout << "DIGADAI oleh " << property.getOwner()->getUsername();
    else
        cout << "Dimiliki oleh " << property.getOwner()->getUsername();
    cout << "\n";

    const auto* street = dynamic_cast<const StreetTile*>(&property);
    if (street) {
        cout << "Color group : " << colorGroupToString(street->getColorGroup()) << "\n";
        cout << "Monopoli    : " << (street->isMonopolyOwned() ? "Ya" : "Tidak") << "\n";
        int lvl = street->getPropertyLevel();
        cout << "Bangunan    : ";
        if (lvl == 0)
            cout << "Kosong";
        else if (lvl == 5)
            cout << "Hotel";
        else
            cout << lvl << " rumah";
        cout << "\n";
        cout << "Harga rumah : M" << street->getHousePrice() << "\n";
        cout << "Harga hotel : M" << street->getHotelPrice() << "\n";
        if (street->hasFestival()) {
            cout << "Festival    : aktif x" << street->getFestivalMultiplier() << ", sisa "
                 << street->getFestivalDur() << " giliran\n";
        }
        cout << "--- Tabel Sewa ---\n";
        const string labels[] = {"Kosong", "1 Rumah", "2 Rumah", "3 Rumah", "4 Rumah", "Hotel"};
        const auto& rt = property.getRentTable();
        for (int i = 0; i < static_cast<int>(rt.size()) && i < 6; ++i) {
            cout << "  " << labels[i] << ": M" << rt[i] << "\n";
        }
    } else {
        cout << "--- Tabel Sewa ---\n";
        const auto& rt = property.getRentTable();
        for (int i = 0; i < static_cast<int>(rt.size()); ++i) {
            cout << "  Level " << i << ": M" << rt[i] << "\n";
        }
    }
    cout << "========================================\n";
}

void GameView::printPlayerProperties(const Player& player) const {
    const auto& props = player.getProperties();
    if (props.empty()) {
        cout << player.getUsername() << " tidak memiliki properti.\n";
        return;
    }
    cout << "=== Properti milik " << player.getUsername() << " ===\n";
    for (const PropertyTile* p : props) {
        cout << "  [" << p->getCode() << "] " << p->getName();
        if (p->isMortgaged())
            cout << " [GADAI]";
        const auto* s = dynamic_cast<const StreetTile*>(p);
        if (s && s->getPropertyLevel() > 0) {
            int lvl = s->getPropertyLevel();
            cout << " [" << (lvl == 5 ? "Hotel" : to_string(lvl) + " Rumah") << "]";
        }
        if (s && s->hasFestival()) {
            cout << " [Festival x" << s->getFestivalMultiplier() << "]";
        }
        cout << " - Harga M" << p->getPrice() << "\n";
    }
    cout << "Total      : " << props.size() << " properti\n";
    cout << "Kekayaan   : M" << player.getTotalWealth() << "\n";
}

void GameView::renderAuctionPanel(int currentBid, const string& highBidder,
                                  const PropertyTile& tile) const {
    cout << "=== LELANG: " << tile.getName() << " [" << tile.getCode() << "] ===\n";
    cout << "Penawaran tertinggi: M" << currentBid << " oleh " << highBidder << "\n";
    cout << "Perintah: BID <jumlah> | PASS\n";
}

string GameView::getCommandInput(const Player& activePlayer) const {
    cout << "\n[" << activePlayer.getUsername() << " | M" << activePlayer.getMoney() << "] > ";
    string line;
    getline(cin, line);
    return line;
}

void GameView::printTransactionLogs(const vector<string>& logs) const {
    if (logs.empty()) {
        cout << "(Log kosong)\n";
        return;
    }
    cout << "=== Log Transaksi ===\n";
    for (const string& entry : logs) {
        cout << entry << "\n";
    }
    cout << "=====================\n";
}

void GameView::showEndGameScreen(const vector<string>& winners,
                                 const vector<string>& finalRankings) const {
    cout << "\n============================================\n";
    cout << "             PERMAINAN SELESAI!             \n";
    cout << "============================================\n";
    cout << "Pemenang: ";
    for (const string& w : winners)
        cout << w << "  ";
    cout << "\n\nPeringkat Akhir:\n";
    for (const string& r : finalRankings)
        cout << "  " << r << "\n";
    cout << "============================================\n";
}
