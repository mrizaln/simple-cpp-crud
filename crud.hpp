#ifndef CRUD_HPP_OLXZWVP0
#define CRUD_HPP_OLXZWVP0

#include "mahasiswa.hpp"
#include "cin_wrapper.hpp"

#include <fstream>
#include <vector>
#include <string_view>

class Crud
{
public:
    enum class Opt
    {
        INVALID = 0,
        CREATE,
        READ,
        UPDATE,
        DELETE,
        FINISH,
    };

    static constexpr std::string_view s_dataFileName = "data.bin";

private:
    /*
     * the data is serialized as follows (Mahasiswa also serialized like this)
     *  - the data is represented as array inside the file
     *  - at the start of the file, 4 bytes (sizeof(int)) used to store the size of the array
     *  - Mahasiswa is serialized as string (see Mahasiswa::serialize())
     *  - each serialized string from Mahasiswa has its size inserted at the start of it
     *    (4 bytes [see: intToBin() in util.hpp])
     */
    std::fstream           m_data;
    CinWrapper             m_cin;
    std::vector<Mahasiswa> m_records;
    int                    m_recordCounter;
    bool                   m_isDataChanged = false;
    bool                   m_terminate     = false;

public:
    Crud();
    Crud(const Crud&)            = delete;
    Crud& operator=(const Crud&) = delete;
    Crud(Crud&&)                 = delete;
    Crud& operator=(Crud&&)      = delete;
    ~Crud();

    void run();

private:
    static std::fstream    initializeDatabase();
    std::vector<Mahasiswa> loadData();
    void                   writeData();

    Opt  getOption();
    void addRecord();                            // CREATE
    void displayRecords(bool prompt = false);    // READ
    void updateRecord();                         // UPDATE
    void deleteRecord();                         // DELETE
};

#endif /* end of include guard: CRUD_HPP_OLXZWVP0 */
