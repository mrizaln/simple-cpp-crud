#ifndef MAHASISWA_HPP_VG5TNJFV
#define MAHASISWA_HPP_VG5TNJFV

#include "util.hpp"

#include <string>
#include <ostream>

struct Mahasiswa
{
    int         m_pk;
    std::string m_nim;
    std::string m_nama;
    std::string m_jurusan;

    /*
     * Serialization:
     *  - integer field is directly copied bit-by-bit to char array (4 bytes [sizeof(int)] using memcpy)
     *  - each string field has its size inserted at the start of the string (encoded as above)
     */
    std::string serialize() const
    {
        auto pk_bin = intToBin(m_pk);

        std::string str{ pk_bin.begin(), pk_bin.end() };
        str += insertSize(m_nim);
        str += insertSize(m_nama);
        str += insertSize(m_jurusan);

        return str;
    }

    static Mahasiswa deserialize(const std::string& string)
    {
        constexpr std::size_t intSize = IntBinType{}.size();

        Mahasiswa   mahasiswa;
        std::size_t offset = 0;

        auto pk_bin  = strToBin(string.substr(offset, offset + intSize));
        offset      += intSize;
        int pk       = binToInt(pk_bin);

        mahasiswa.m_pk = pk;

        auto nim_size  = binToInt(strToBin(string.substr(offset, intSize)));
        offset        += intSize;
        auto nim       = string.substr(offset, static_cast<std::size_t>(nim_size));
        offset        += nim.size();

        mahasiswa.m_nim = nim;

        auto nama_size  = binToInt(strToBin(string.substr(offset, intSize)));
        offset         += intSize;
        auto nama       = string.substr(offset, static_cast<std::size_t>(nama_size));
        offset         += nama.size();

        mahasiswa.m_nama = nama;

        auto jurusan_size  = binToInt(strToBin(string.substr(offset, intSize)));
        offset            += intSize;
        auto jurusan       = string.substr(offset, static_cast<std::size_t>(jurusan_size));
        offset            += jurusan.size();

        mahasiswa.m_jurusan = jurusan;

        return mahasiswa;
    }

    bool operator==(const Mahasiswa& other) const
    {
        // clang-format off
        return m_pk == other.m_pk
            && m_nim == other.m_nim
            && m_nama == other.m_nama
            && m_jurusan == other.m_jurusan;
        // clang-format on
    }

    friend std::ostream& operator<<(std::ostream& os, const Mahasiswa& mahasiswa)
    {
        os << "Mahasiswa:\n";
        os << "\tPK: " << mahasiswa.m_pk << '\n';
        os << "\tNIM: " << mahasiswa.m_nim << '\n';
        os << "\tNama: " << mahasiswa.m_nama << '\n';
        os << "\tJurusan: " << mahasiswa.m_jurusan << '\n';
        return os;
    }
};

#endif /* end of include guard: MAHASISWA_HPP_VG5TNJFV */
