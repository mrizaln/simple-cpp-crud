#include "crud.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ios>
#include <iostream>
#include <limits>
#include <thread>

using namespace std::chrono_literals;

namespace
{
    void printWithDelay(const std::string& str, std::chrono::milliseconds delayTime = 500ms)
    {
        std::cout << str << std::flush;
        std::this_thread::sleep_for(delayTime);
    }
}

Crud::Crud()
    : m_data{ initializeDatabase() }
    , m_cin{ [this] {
        std::cout << "\nEOF eaten! Skipping input...\n";
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
    while (!m_terminate) {
        switch (getOption()) {
        case Opt::INVALID: {
            printWithDelay("Pilihan tidak valid\n");
            break;
        }
        case Opt::CREATE: {
            std::cout << "\nMenambah data mahasiswa\n";
            promptCreate();
            break;
        }
        case Opt::READ: {
            std::cout << "\nTampilkan data mahasiswa\n";
            promptRead();
            break;
        }
        case Opt::UPDATE: {
            std::cout << "\nUbah data mahasiswa\n";
            promptUpdate();
            break;
        }
        case Opt::DELETE: {
            std::cout << "\nHapus data mahasiswa\n";
            promptDelete();
            break;
        }
        case Opt::FINISH: {
            std::cout << "\nSelesai\n";
            m_terminate = true;     // why not use the flag :shrug:
            break;
        }
        }
    }
    std::cout << "\nProgram selesai. Terima kasih!" << std::endl;   // endl flushes the stream
}

std::fstream Crud::initializeDatabase()
{
    std::fstream file{ s_dataFileName.data(), std::ios::out | std::ios::in | std::ios::binary };

    // check file ada atau tidak
    if (file.is_open()) {
        printWithDelay("Database ditemukan\n");
    } else {
        file.close();
        file.open(s_dataFileName.data(), std::ios::trunc | std::ios::out | std::ios::in | std::ios::binary);

        const int  initialSize    = 0;
        IntBinType initialSizeBin = intToBin(initialSize);
        file.write(initialSizeBin.data(), initialSizeBin.size());

        printWithDelay("Database tidak ditemukan, buat database baru\n");
    }

    return file;
}

Crud::Opt Crud::getOption()
{
    // clear screen
    std::cout << "\033[H\033[2J";

    std::cout << "| Program CRUD data mahasiswa |\n"
                 "| --------------------------- |\n"
                 "| 1. Tambah data mahasiswa    |\n"
                 "| 2. Tampilkan data mahasiswa |\n"
                 "| 3. Ubah data mahasiswa      |\n"
                 "| 4. Hapus data mahasiswa     |\n"
                 "| 5. Selesai                  |\n\n";

    std::cout << ">>> Pilih [1-5]: ";

    if (m_terminate) {
        return Opt::FINISH;
    }

    using Int = std::underlying_type_t<Opt>;
    auto maybeInput = m_cin.getFromLine<Int>();
    if (!maybeInput) {
        return Opt::INVALID;
    }

    int input = *maybeInput;
    if (input < 1 || input > static_cast<Int>(Opt::FINISH)) {
        return Opt::INVALID;
    }

    return static_cast<Opt>(input);
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

void Crud::displayRecords()
{
    // string formatting before std::format is such a hassle, sigh...
    constexpr std::size_t w_d    = 3;
    constexpr std::size_t w_s    = 30;
    constexpr const char* format = "| %%%zu%c | %%%zu%c | %%-%zu.%zus | %%-%zu.%zus | %%-%zu.%zus |\n";
    const std::size_t     w_f    = std::strlen(format);    // for format buffer, won't be longer than above

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
}

void Crud::promptCreate()
{
    // save cursor position
    std::cout << "\033[s";

    while (true) {
        // restore cursor position and clear to the end of screen
        std::cout << "\033[u\033[J";

        displayRecords();

        Mahasiswa inputMahasiswa;
        auto& [pk, nim, nama, jurusan]{ inputMahasiswa };

        std::cout << "\nUkuran data : " << m_records.size() << '\n';

        // clang-format off
        std::cout << "\n> Input data baru (kosongkan semua field/bidang untuk selesai)\n";
        std::cout << "    PK     : " << (pk = m_recordCounter++) << '\n';
        std::cout << ">>> Nama   : "; m_cin.getline(nama);
        std::cout << ">>> Jurusan: "; m_cin.getline(jurusan);
        std::cout << ">>> NIM    : "; m_cin.getline(nim);
        // clang-format on

        if (nama.empty() && jurusan.empty() && nim.empty()) {
            break;
        }

        if (m_terminate) {
            return;
        }

        m_records.push_back(inputMahasiswa);

        // 5 lines up, calculated manually because the save cursor "buffer" is used already
        std::cout << "\033[5F\033[J";
        printWithDelay("> Data berhasil ditambahkan!\n");

        m_isDataChanged = true;
    }
}

void Crud::promptRead()
{
    displayRecords();

    // save cursor position
    std::cout << "\033[s";

    while (true) {
        // restore cursor position
        std::cout << "\033[u";

        std::cout << "\n>>> Pilih PK untuk melihat detail (0 untuk batal): ";
        auto maybePk = m_cin.getFromLine<int>();

        // go to previous line then clear it to the end of screen
        std::cout << "\033[F\033[J";

        if (m_terminate) {
            break;
        }

        if (!maybePk) {
            printWithDelay("> Input tidak valid\n");
            continue;
        }
        int pk = *maybePk;

        if (pk == 0) {
            break;
        }

        auto found = std::find_if(m_records.begin(), m_records.end(), [&pk](const auto& record) {
            return record.m_pk == static_cast<int>(pk);
        });

        if (found == m_records.end()) {
            printWithDelay("> Item tidak ditemukan\n");
            continue;
        }

        const auto& [_, nim, nama, jurusan]{ *found };
        std::cout << "\n> Detail data untuk PK " << pk << '\n';
        std::cout << "\tPK     : " << pk << '\n';
        std::cout << "\tNama   : " << nama << '\n';
        std::cout << "\tJurusan: " << jurusan << '\n';
        std::cout << "\tNIM    : " << nim << '\n';
    }
}

void Crud::promptUpdate()
{
    // save cursor position
    std::cout << "\033[s";

    while (true) {
        // restore cursor position and clear to the end of screen
        std::cout << "\033[u\033[J";

        displayRecords();

        std::cout << "\n>>> Pilih PK untuk diubah (0 untuk batal): ";
        auto maybePk = m_cin.getFromLine<int>();

        if (m_terminate) {
            return;
        }

        // go to previous line then clear it to the end of screen
        std::cout << "\033[F\033[J";

        if (!maybePk) {
            printWithDelay("> Input tidak valid\n");
            continue;
        }
        int pk = *maybePk;

        auto record = std::find_if(m_records.begin(), m_records.end(), [pk](const auto& record) {
            return record.m_pk == static_cast<int>(pk);
        });

        if (pk == 0) {
            return;
        }

        if (record == m_records.end()) {
            printWithDelay("> Item tidak ditemukan\n");
            continue;
        }

        std::cout << "> Detail data terpilih\n";
        std::cout << "\tPK     : " << pk << '\n';
        std::cout << "\tNama   : " << record->m_nama << '\n';
        std::cout << "\tJurusan: " << record->m_jurusan << '\n';
        std::cout << "\tNIM    : " << record->m_nim << '\n';

        Mahasiswa temporary{ *record };
        auto& [_, nim, nama, jurusan]{ temporary };

        const auto ifEmpty = [](std::string& str, const std::string& replacement) {
            str = str.empty() ? replacement : str;
        };

        std::cout << "\n> Merubah data (kosongkan field/bidang jika tidak ingin merubahnya)\n";
        // clang-format off
        std::cout << "    PK     : " << pk << '\n';
        std::cout << ">>> Nama   : "; m_cin.getline(nama);    ifEmpty(nama,    record->m_nama);
        std::cout << ">>> Jurusan: "; m_cin.getline(jurusan); ifEmpty(jurusan, record->m_jurusan);
        std::cout << ">>> NIM    : "; m_cin.getline(nim);     ifEmpty(nim,     record->m_nim);
        // clang-format on

        if (m_terminate) {
            return;
        }

        *record = temporary;    // copy the temporary data to the record

        // 11 lines up, calculated manually because the save cursor "buffer" is used already
        std::cout << "\033[11F\033[J";
        printWithDelay("> Data berhasil diubah!\n");

        m_isDataChanged = true;
    }
}

void Crud::promptDelete()
{
    // save cursor position
    std::cout << "\033[s";

    while (true) {
        // restore cursor position and clear to the end of screen
        std::cout << "\033[u\033[J";

        displayRecords();

        std::cout << "\n>>> Pilih PK untuk dihapus (0 untuk batal): ";
        auto maybePk = m_cin.getFromLine<int>();

        // go to previous line then clear it to the end of screen
        std::cout << "\033[F\033[J";

        if (m_terminate) {
            return;
        }

        if (!maybePk) {
            printWithDelay("> Input tidak valid\n");
            continue;
        }
        int pk = *maybePk;

        if (pk == 0) {
            return;
        }

        auto found = std::find_if(m_records.begin(), m_records.end(), [&pk](const auto& record) {
            return record.m_pk == static_cast<int>(pk);
        });

        if (found == m_records.end()) {
            printWithDelay("> Item tidak ditemukan\n");
            continue;
        }

        m_records.erase(found);

        printWithDelay("> Data berhasil dihapus!\n");

        m_isDataChanged = true;
    }
}
