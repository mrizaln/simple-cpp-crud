#include "crud.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ios>
#include <iostream>
#include <limits>
#include <thread>

// go to the start of the screen and clear the screen
#define DO_RESET_SCREEN 1

Crud::Crud()
    : m_data{ initializeDatabase() }
    , m_cin{ [this] {
        std::cout << "\nEOF eaten! Terminating...\n";
        m_terminate = true;
    } }
    , m_records{ loadData() }
    , m_recordCounter{ 1 }
{
    if (!m_records.empty()) {
        m_recordCounter = m_records.back().m_pk + 1;
    }
}

Crud::~Crud()
{
    if (m_isDataChanged) {
        writeData();
    }
}

void Crud::run()
{
    Crud::Opt pilihan = Crud::Opt::INVALID;
    while (pilihan != Opt::FINISH) {

    label_retry:;

        pilihan = getOption();

        switch (pilihan) {
        case Opt::CREATE: {
            std::cout << "\nMenambah data mahasiswa\n";
            displayRecords();
            addRecord();
            break;
        }
        case Opt::READ: {
            std::cout << "\nTampilkan data mahasiswa\n";
            displayRecords(true);
            break;
        }
        case Opt::UPDATE: {
            std::cout << "\nUbah data mahasiswa\n";
            displayRecords();
            updateRecord();
            break;
        }
        case Opt::DELETE: {
            std::cout << "\nHapus data mahasiswa\n";
            displayRecords();
            deleteRecord();
            break;
        }
        case Opt::FINISH: {
            goto label_done;
            break;
        }
        case Opt::INVALID: [[fallthrough]];
        default: {
            std::cout << "Pilihan tidak ditemukan\n";
            constexpr auto delayTime = std::chrono::milliseconds(500);
            std::this_thread::sleep_for(delayTime);
            if (!m_terminate) {
                goto label_retry;
            }
            break;
        }
        }

    label_continue:;

        if (m_terminate) {
            break;
        }

        char is_continue;
        std::cout << "\n>>> Lanjutkan?(y/n) : ";
        m_cin >> is_continue;

        if ((is_continue == 'y') || (is_continue == 'Y')) {
            goto label_retry;
        } else if ((is_continue == 'n') || (is_continue == 'N')) {
            break;
        } else {
            goto label_continue;
        }

    label_done:;
    }

    std::cout << "\nSelesai\n";
}

std::fstream Crud::initializeDatabase()
{
    std::fstream file{ s_dataFileName.data(), std::ios::out | std::ios::in | std::ios::binary };

    // check file ada atau tidak
    if (file.is_open()) {
        std::cout << "Database ditemukan\n";

        // just so the user can see whether the database is exist or not
        constexpr auto delayTime = std::chrono::milliseconds(500);
        std::this_thread::sleep_for(delayTime);
    } else {
        std::cout << "Database tidak ditemukan, buat database baru\n";
        file.close();
        file.open(s_dataFileName.data(), std::ios::trunc | std::ios::out | std::ios::in | std::ios::binary);

        const int  initialSize    = 0;
        IntBinType initialSizeBin = intToBin(initialSize);
        file.write(initialSizeBin.data(), initialSizeBin.size());

        // a little longer for this one
        constexpr auto delayTime = std::chrono::milliseconds(1'000);
        std::this_thread::sleep_for(delayTime);
    }

    return file;
}

Crud::Opt Crud::getOption()
{
    int input{ 0 };

#if DO_RESET_SCREEN
    std::cout << "\033[H\033[2J";
#endif

    std::cout << "| Program CRUD data mahasiswa |\n"
                 "| --------------------------- |\n"
                 "| 1. Tambah data mahasiswa    |\n"
                 "| 2. Tampilkan data mahasiswa |\n"
                 "| 3. Ubah data mahasiswa      |\n"
                 "| 4. Hapus data mahasiswa     |\n"
                 "| 5. Selesai                  |\n\n";

    std::cout << ">>> Pilih [1-5]: ";
    m_cin >> input;

    if (m_terminate) {
        return Opt::FINISH;
    }

    if (input > 0 && input <= static_cast<int>(Opt::FINISH)) {
        return static_cast<Opt>(input);
    }

    return Opt::INVALID;
}

void Crud::writeData()
{
    // set the size
    std::size_t size    = m_records.size();
    auto        sizeBin = intToBin(static_cast<int>(size));

    std::string serialized{ sizeBin.begin(), sizeBin.end() };
    for (const auto& mahasiswa : m_records) {
        // serialize the data
        auto str = mahasiswa.serialize();
        str      = insertSize(str);

        serialized += str;
    }

    m_data.seekp(0, std::ios::beg);
    m_data.write(serialized.c_str(), static_cast<std::streamsize>(serialized.size()));
}

std::vector<Mahasiswa> Crud::loadData()
{
    const auto getDataSize = [this] {
        m_data.seekg(0, std::ios::beg);
        IntBinType sizeBin;
        m_data.read(sizeBin.data(), sizeBin.size());
        return binToInt(sizeBin);
    };
    if (getDataSize() == 0) {
        return {};
    }

    std::vector<Mahasiswa> records;
    int                    recordsSize = getDataSize();
    int                    offset      = IntBinType{}.size();

    // iterate through all data (Mahasiswa)
    int counter = 0;
    while (counter < recordsSize) {
        IntBinType dataSizeBin;
        m_data.read(dataSizeBin.data(), dataSizeBin.size());
        int dataSize  = binToInt(dataSizeBin);
        offset       += dataSizeBin.size();

        // read the data
        m_data.seekg(offset, std::ios::beg);
        std::string serialized;
        serialized.resize(static_cast<std::size_t>(dataSize));
        m_data.read(serialized.data(), static_cast<std::streamsize>(serialized.size()));

        auto mahasiswa = Mahasiswa::deserialize(serialized);
        records.push_back(mahasiswa);

        offset += dataSize;

        counter++;
    }

    std::sort(records.begin(), records.end(), [](const auto& left, const auto& right) {
        return left.m_pk < right.m_pk;
    });

    m_data.seekg(0, std::ios::beg);

    return records;
}

void Crud::addRecord()
{
    Mahasiswa inputMahasiswa;
    auto& [pk, nim, nama, jurusan]{ inputMahasiswa };

    std::cout << "Ukuran data : " << m_records.size() << '\n';

    // clang-format off
    std::cout << "\nInput data baru\n";
    std::cout << "    PK      : " << (pk = m_recordCounter++) << '\n';
    std::cout << ">>> Nama    : "; m_cin.getline(nama);
    std::cout << ">>> Jurusan : "; m_cin.getline(jurusan);
    std::cout << ">>> NIM     : "; m_cin.getline(nim);
    // clang-format on

    if (m_terminate) {
        return;
    }

    m_records.push_back(inputMahasiswa);

    m_isDataChanged = true;
}

void Crud::displayRecords(bool prompt)
{
    // string formatting before std::format is such a hassle, sigh...
    constexpr std::size_t w_d    = 3;
    constexpr std::size_t w_s    = 30;
    constexpr const char* format = "| %%%zu%c | %%%zu%c | %%-%zu.%zus | %%-%zu.%zus | %%-%zu.%zus |\n";
    const std::size_t     w_f    = std::strlen(format);    // for format bufffer, won't be longer than above

    const auto t = [&](const std::string& str) {
        if (str.size() > w_s) {
            return str.substr(0, w_s - 3) + "...";
        }
        return str;
    };

    std::string sep_fmt(w_f, '\0');
    std::sprintf(sep_fmt.data(), format, w_d, 's', w_d, 's', w_s, w_s, w_s, w_s, w_s, w_s);

    std::string record_fmt(w_f, '\0');
    std::sprintf(record_fmt.data(), format, w_d, 'd', w_d, 'd', w_s, w_s, w_s, w_s, w_s, w_s);

    std::printf(sep_fmt.c_str(), "No", "PK", "NIM", "Nama", "Jurusan");

    const auto s   = [](std::size_t w) { return std::string(w, '-'); };
    const auto s_d = s(w_d);
    const auto s_s = s(w_s);
    std::printf(sep_fmt.c_str(), s_d.c_str(), s_d.c_str(), s_s.c_str(), s_s.c_str(), s_s.c_str());

    for (std::size_t i{ 0 }; i < m_records.size(); ++i) {
        const auto& [pk, nim, nama, jurusan]{ m_records.at(i) };
        std::printf(record_fmt.c_str(), i + 1, pk, t(nim).c_str(), t(nama).c_str(), t(jurusan).c_str());
    }

    if (!prompt) {
        return;
    }

    // save cursor position
    std::cout << "\033[s";

    while (true) {
        // restore cursor position
        std::cout << "\033[u";

        int pk;
        std::cout << "\n>>> Pilih PK untuk melihat detail (0 untuk batal): ";
        m_cin >> pk;

        // go to previous line then clear it to the end of screen, then go to the next line
        std::cout << "\033[F\033[J\033[E";

        if (m_cin.previousFail()) {
            std::cout << "> Input tidak valid\n";
            continue;
        }

        if (m_terminate || pk == 0) {
            break;
        }

        auto found = std::find_if(m_records.begin(), m_records.end(), [&pk](const auto& record) {
            return record.m_pk == static_cast<int>(pk);
        });

        if (found == m_records.end()) {
            std::cout << "> Item dengan PK " << pk << " tidak ditemukan\n";
            continue;
        }

        const auto& [_, nim, nama, jurusan]{ *found };
        std::cout << "> Detail data untuk PK " << pk << '\n';
        std::cout << "\tPK     : " << pk << '\n';    // redundant, but for consistency
        std::cout << "\tNama   : " << nama << '\n';
        std::cout << "\tJurusan: " << jurusan << '\n';
        std::cout << "\tNIM    : " << nim << '\n';
    }
}

void Crud::updateRecord()
{
label_retry:
    int pk;
    std::cout << "\n>>> Pilih PK (0 untuk batal): ";
    m_cin >> pk;

    if (m_cin.previousFail()) {
        std::cout << "Input tidak valid\n";
        goto label_retry;
    }

    auto record = std::find_if(m_records.begin(), m_records.end(), [pk](const auto& record) {
        return record.m_pk == static_cast<int>(pk);
    });

    if (m_terminate || pk == 0) {
        return;
    }

    if (record == m_records.end()) {
        std::cout << "Item dengan PK" << pk << " tidak ditemukan\n";
        goto label_retry;
    }

    std::cout << "\n\npilihan data: " << '\n';
    std::cout << "Nama   : " << record->m_nama << '\n';
    std::cout << "Jurusan: " << record->m_jurusan << '\n';
    std::cout << "NIM    : " << record->m_nim << '\n';

    std::cout << "\nMerubah data: \n";
    // clang-format off
    std::cout << ">>> Nama   : "; m_cin.getline(record->m_nama);
    std::cout << ">>> Jurusan: "; m_cin.getline(record->m_jurusan);
    std::cout << ">>> NIM    : "; m_cin.getline(record->m_nim);
    // clang-format on

    if (m_terminate) {
        return;
    }

    m_isDataChanged = true;
}

void Crud::deleteRecord()
{
label_retry:
    int pk;
    std::cout << "\n>>> Pilih PK untuk dihapus (0 untuk batal): ";
    m_cin >> pk;

    if (m_cin.previousFail()) {
        std::cout << "Input tidak valid\n";
        goto label_retry;
    }

    if (m_terminate || pk == 0) {
        return;
    }

    auto found = std::find_if(m_records.begin(), m_records.end(), [&pk](const auto& record) {
        return record.m_pk == static_cast<int>(pk);
    });

    if (found == m_records.end()) {
        std::cout << "Item dengan PK " << pk << " tidak ditemukan\n";
        goto label_retry;
    } else {
        m_records.erase(found);
        std::cout << "Item dengan PK " << pk << " dihapus\n";
    }

    m_isDataChanged = true;
}
