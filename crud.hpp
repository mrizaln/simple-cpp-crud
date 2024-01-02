#ifndef CRUD_HPP_OLXZWVP0
#define CRUD_HPP_OLXZWVP0

#include "mahasiswa.hpp"

#include <fstream>
#include <vector>

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

    static constexpr std::string s_dataFileName = "data.bin";

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
    std::vector<Mahasiswa> m_records;
    bool                   m_isDataChanged = false;
    int                    m_recordCounter;

public:
    Crud();
    Crud(const Crud&)            = delete;
    Crud& operator=(const Crud&) = delete;
    Crud(Crud&&)                 = default;
    Crud& operator=(Crud&&)      = default;
    ~Crud();

    void run();

private:
    static std::fstream    initializeDatabase();
    std::vector<Mahasiswa> loadData();
    void                   writeData();

    Opt  getOption();
    void addRecord();        // CREATE
    void displayRecord();    // READ
    void updateRecord();     // UPDATE
    void deleteRecord();     // DELETE
};

#endif /* end of include guard: CRUD_HPP_OLXZWVP0 */
