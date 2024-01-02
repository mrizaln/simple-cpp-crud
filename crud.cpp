#include "crud.hpp"

#include <algorithm>
#include <ios>
#include <iostream>
#include <limits>

Crud::Crud()
    : m_data{ initializeDatabase() }
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
    label_retry:
        pilihan = getOption();

        switch (pilihan) {
        case Opt::CREATE: {
            std::cout << "Menambah data mahasiswa\n";
            addRecord();
            break;
        }
        case Opt::READ: {
            std::cout << "Tampilkan data mahasiswa\n";
            displayRecord();
            break;
        }
        case Opt::UPDATE: {
            std::cout << "Ubah data mahasiswa\n";
            updateRecord();
            break;
        }
        case Opt::DELETE: {
            std::cout << "Hapus data mahasiswa\n";
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
            goto label_retry;
            break;
        }
        }

    label_continue:

        char is_continue;
        std::cout << "Lanjutkan?(y/n) : ";
        std::cin >> is_continue;
        if (std::cin.fail()) {
            std::cin.clear();
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if ((is_continue == 'y') || (is_continue == 'Y')) {
            goto label_retry;
        } else if ((is_continue == 'n') || (is_continue == 'N')) {
            break;
        } else {
            goto label_continue;
        }

    label_done:;
    }
}

std::fstream Crud::initializeDatabase()
{
    std::fstream file{ s_dataFileName.data(), std::ios::out | std::ios::in | std::ios::binary };

    // check file ada atau tidak
    if (file.is_open()) {
        std::cout << "database ditemukan\n";
    } else {
        std::cout << "database tidak ditemukan, buat database baru\n";
        file.close();
        file.open(s_dataFileName.data(), std::ios::trunc | std::ios::out | std::ios::in | std::ios::binary);

        const int  initialSize    = 0;
        IntBinType initialSizeBin = intToBin(initialSize);
        file.write(initialSizeBin.data(), initialSizeBin.size());
    }

    return file;
}

Crud::Opt Crud::getOption()
{
    int input;

    // system("clear");
    // system("cls");

    std::cout << "\nProgram CRUD data mahasiswa\n";
    std::cout << "=============================\n";
    std::cout << "1. Tambah data mahasiswa\n";
    std::cout << "2. Tampilkan data mahasiswa\n";
    std::cout << "3. Ubah data mahasiswa\n";
    std::cout << "4. Hapus data mahasiswa\n";
    std::cout << "5. Selesai\n";
    std::cout << "=============================\n";
    std::cout << "pilih [1-5]? : ";
    std::cin >> input;

    if (std::cin.fail()) {
        std::cin.clear();
        input = 0;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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

    std::cout << "ukuran data : " << m_records.size() << '\n';

    inputMahasiswa.m_pk = m_recordCounter++;
    std::cout << "pk = " << inputMahasiswa.m_pk << '\n';

    std::cout << "Nama: ";
    getline(std::cin, inputMahasiswa.m_nama);
    std::cout << "Jurusan: ";
    getline(std::cin, inputMahasiswa.m_jurusan);
    std::cout << "NIM: ";
    getline(std::cin, inputMahasiswa.m_nim);

    m_records.push_back(inputMahasiswa);

    m_isDataChanged = true;
}

void Crud::displayRecord()
{
    for (const auto& mahasiswa : m_records) {
        std::cout << mahasiswa << '\n';
    }
}

void Crud::updateRecord()
{
label_retry:
    std::size_t nomor;
    std::cout << "pilih no: ";
    std::cin >> nomor;

    if (std::cin.fail()) {
        std::cin.clear();
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    auto record = std::find_if(m_records.begin(), m_records.end(), [nomor](const auto& record) {
        return record.m_pk == static_cast<int>(nomor);
    });

    if (record == m_records.end()) {
        std::cout << "record nomor " << nomor << " tidak ditemukan\n";
        goto label_retry;
    }

    std::cout << "\n\npilihan data: " << '\n';
    std::cout << "NIM : " << record->m_nim << '\n';
    std::cout << "nama : " << record->m_nama << '\n';
    std::cout << "jurusan : " << record->m_jurusan << '\n';

    std::cout << "\nMerubah data: \n";
    std::cout << "NIM: ";
    std::getline(std::cin, record->m_nim);
    std::cout << "nama: ";
    std::getline(std::cin, record->m_nama);
    std::cout << "jurusan: ";
    std::getline(std::cin, record->m_jurusan);

    m_isDataChanged = true;
}

void Crud::deleteRecord()
{
label_retry:
    std::size_t nomor;
    std::cout << "Hapus nomor: ";
    std::cin >> nomor;

    if (std::cin.fail()) {
        std::cin.clear();
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    auto found = std::find_if(m_records.begin(), m_records.end(), [&nomor](const auto& record) {
        return record.m_pk == static_cast<int>(nomor);
    });

    if (found == m_records.end()) {
        std::cout << "item nomor " << nomor << " tidak ditemukan\n";
        goto label_retry;
    } else {
        m_records.erase(found);
        std::cout << "item nomor " << nomor << " dihapus\n";
    }

    m_isDataChanged = true;
}
