#ifndef HB_OPEN_FILE_PRIVATE_H
#define HB_OPEN_FILE_PRIVATE_H

// Organization of an OpenType Font
// note: structs are supposely properly aligned to avoid the use of packing


#define IDX_NOT_FOUND 0xffff

struct tbl_rec
{
  uint32_t tag;    //4-byte identifier
  uint32_t chk_sum;//Check sum for this table
  uint32_t of;     //Offset from beginning of TrueType font file
  uint32_t len;    //Length of this table
};
#define ot_tbl tbl_rec

struct ot_fnt_face
{
  uint32_t sfnt_ver;      //'\0\001\0\00' if TrueType / 'OTTO' if CFF
  uint16_t tbls_n;        //Number of tables
  uint16_t search_range;  //(Maximum power of 2 <= tbls_n) * 16
  uint16_t entry_selector;//Log2(maximum power of 2 <= tbls_n)
  uint16_t range_shift;   //tbls_n * 16 - search_range
  struct tbl_rec tbls[1]; //tbl_rec entries. tbls_n items.
};
#define of_tbl ot_fnt_face

struct fixed_ver
{
  uint16_t major;
  uint16_t minor;
};

struct ttc_hdr_ver1
{
  uint32_t ttc_tag;       //TrueType Collection ID string: 'ttcf'
  struct fixed_ver ver;   //Version of the TTC Header (1.0), 0x00010000 
  uint32_t of_tbls_ofs[1];//Array of offsets to the OffsetTable for each
                          //font from the beginning of the file
};

struct ttc_hdr
{
  union {
    struct {
      uint32_t ttc_tag;    //TrueType Collection ID string: 'ttcf'
      struct fixed_ver ver;//Version of the TTC Header (1.0 or 2.0),
                           //* 0x00010000 or 0x00020000 */
    } hdr;
    struct ttc_hdr_ver1 ver1;
  } u;
};

//OpenType with Postscript outlines
#define CFF_TAG HB_TAG('O','T','T','O')
//OpenType with TrueType outlines
#define TRUETYPE_TAG HB_TAG( 0 , 1 , 0 , 0 )
//TrueType Collection
#define TTC_TAG HB_TAG('t','t','c','f')
//Obsolete Apple TrueType
#define TRUE_TAG HB_TAG('t','r','u','e')
//Obsolete Apple Type1 font in SFNT container
#define TYP1_TAG HB_TAG('t','y','p','1')

struct ot_fnt_file
{
  union {
    uint32_t tag;
    struct ot_fnt_face fnt_face;
    struct ttc_hdr ttc_hdr;
  } u;
};

struct ot_fnt_face *
ot_fnt_file_get_face(struct ot_fnt_file *ot_fnt_file, unsigned i);

struct tbl_rec *
ot_fnt_face_get_tbl_by_tag(struct ot_fnt_face *ot_fnt_face, hb_tag_t tag);
#endif
