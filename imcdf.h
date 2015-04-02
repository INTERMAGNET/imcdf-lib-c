/*****************************************************************************
 * imcdf.h - include file for the C version of ImagCDF - the INTERMAGNET
 *           CDF format
 *
 * THE IMCDF ROUTINES SHOULD NOT HAVE DEPENDENCIES ON OTHER LIBRARY ROUTINES -
 * IT MUST BE POSSIBLE TO DISTRIBUTE THE IMCDF SOURCE CODE
 *
 * Simon Flower, 18/12/2012
 * Updates to version 1.1 of ImagCDF. Simon Flower, 19/02/2015
 *****************************************************************************/

#include "cdf.h"

/* enumerations for the metadata elements that hold structured text */
enum IMCDFPubLevel {IMCDF_PUBLEVEL_1=1, IMCDF_PUBLEVEL_2=2, IMCDF_PUBLEVEL_3=3, IMCDF_PUBLEVEL_4=4};
enum IMCDFStandardLevel {IMCDF_STANDLEVEL_FULL, IMCDF_STANDLEVEL_PARTIAL, IMCDF_STANDLEVEL_NONE};

/* an enumeration listing the type of variables that can be held in an ImagCDF */
enum IMCDFVariableType {IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT, IMCDF_VARTYPE_TEMPERATURE,
                        IMCDF_VARTYPE_ERROR};

/* an enumeration for CDF creation codes when opening a CDF file:
 *     FORCE_CREATE - create the CDF, deleting any existing file;
 *     CREATE - create the CDF, but don't delete any existing file -
 *              an existing file will cause an error;
 *     OPEN - open the CDF, which must already exist */
enum IMCDFOpenType { IMCDF_FORCE_CREATE, IMCDF_CREATE, IMCDF_OPEN };

/* an enumeration for CDF compression types:
 *     NONE - no compression
 *     RLE - run length encoding
 *     HUFF - Huffman encoding
 *     AHUFF - Adaptive Huffman encoding
 *     GZIP<n> - GZIP level 1 to 9 (9 = greatest compression) */
enum IMCDFCompressionType {IMCDF_COMPRESS_NONE, IMCDF_COMPRESS_RLE,
                           IMCDF_COMPRESS_HUFF, IMCDF_COMPRESS_AHUFF,
                           IMCDF_COMPRESS_GZIP1, IMCDF_COMPRESS_GZIP2,
                           IMCDF_COMPRESS_GZIP3, IMCDF_COMPRESS_GZIP4,
                           IMCDF_COMPRESS_GZIP5, IMCDF_COMPRESS_GZIP6,
                           IMCDF_COMPRESS_GZIP7, IMCDF_COMPRESS_GZIP8,
                           IMCDF_COMPRESS_GZIP9};

/* the value used to represent missing data */
#define IMCDF_MISSING_DATA_VALUE 99999.0

/* names of time stamp variables in the CDF file */
#define VECTOR_TIME_STAMPS_VAR_NAME             "GeomagneticVectorTimes"
#define SCALAR_TIME_STAMPS_VAR_NAME             "GeomagneticScalarTimes"
#define TEMPERATURE_TIME_STAMPS_VAR_NAME_BASE   "Temperature%sTimes"

/* a structure that holds the ImagCDF global attributes
 * NOTES:
 * 1.) CDF uses the C 'long long' data type to hold 8-byte integers such at the TT2000 date/time
 *     There are a number of useful conversion routines in the CDF C library to convert to/from
 *     more human readable time formats: CDF_TT2000_from_UTC_parts; CDF_TIME_to_UTC_parts
 *     CDF_TT2000_to_UTC_string; CDF_TT2000_from_UTC_string
 * 2.) Where the IMAG CDF specification allows for more than one attribute with a given name (such
 *     as ParentIdentifiers) the C implementation of this is a pointer to an array of pointers to
 *     a character array. You can declare this as:
 *         char parent_idents [N_ENTRIES] [ENTRY_LEN]; */
struct IMCDFGlobalAttr
{
    /* DD 22, section 4.1 */
    const char *format_description;
    const char *format_version;
    const char *title;
    /* DD 22, section 4.2 */
    const char *iaga_code;
    const char *elements_recorded;                    /* NEW in version 1.1 */
    enum IMCDFPubLevel pub_level;               /* NEW in version 1.1 */
    long long pub_date;                         /* changed data type in version 1.1 - see note 1 */
    /* DD 22, section 4.3 */
    const char *observatory_name;
    double latitude;
    double longitude;
    double elevation;
    const char *institution;
    const char *vector_sens_orient;
    /* DD 22, section 4.3 */
    enum IMCDFStandardLevel standard_level;     /* NEW in version 1.1 */
    const char *standard_name;                        /* NEW in version 1.1 */
    const char *standard_version;                     /* NEW in version 1.1 */
    const char *partial_stand_desc;                   /* NEW in version 1.1 */
    /* DD 22, section 4.4 */
    const char *source;
    const char *terms_of_use;
    const char *unique_identifier;                    /* NEW in version 1.1 */
    char **parent_identifiers;                  /* NEW in version 1.1 - see note 2 */
    int n_parent_identifiers;
    char **reference_links;                     /* NEW in version 1.1 - see note 2 */
    int n_reference_links;
    /* elements that were in use in version 1.0, but have now been removed: */
    /* char *history; */
    /* char *references; */
    /* enum IMCDFBaseline baseline_type; */
    /* enum IMCDFPubState pub_state; */
    /* enum IMCDFStandards standards_conformance; */
};

/* a structure that holds an ImagCDF time stamp array */
struct IMCDFVariableTS
{
    char *var_name;
    long long *time_stamps;
    int data_len;
};

/* a structure that holds an ImagCDF variable along with it's metadata */
struct IMCDFVariable
{
    /* the type of variable and the element recorded */
    enum IMCDFVariableType var_type;
    char elem_rec [10];
    /* the metadata */
    const char *field_nam;
    const char *units;
    double fill_val;
    double valid_min;
    double valid_max;
    char *depend_0;                             /* NEW in version 1.1 */
    /* the data */
    double *data;
    int data_len;
    /* elements that were in use in version 1.0, but have now been removed: */
    /* int var_num; */
    /* double start_date; */
    /* double samp_per; */
    /* char *elem_rec; */
    /* double orig_freq; */
};

/* forward declarations */
/* imcdf.c */
char *imcdf_open2 (const char *filename, enum IMCDFOpenType open_type,
                   enum IMCDFCompressionType compress_type, int *cdf_handle);
char *imcdf_close2 (int cdf_handle);
char *imcdf_read_global_attrs (int cdf_handle, struct IMCDFGlobalAttr *global_attrs);
char *imcdf_read_variable (int cdf_handle, enum IMCDFVariableType var_type,
                           char *elem_rec, struct IMCDFVariable *variable);
char *imcdf_read_time_stamps (int cdf_handle, char *var_name, struct IMCDFVariableTS *ts);
void imcdf_free_global_attrs (struct IMCDFGlobalAttr *global_attrs);
void imcdf_free_variable (struct IMCDFVariable *variable);
void imcdf_free_time_stamps (struct IMCDFVariableTS *ts);
char *imcdf_write_global_attrs (int cdf_handle, struct IMCDFGlobalAttr *global_attrs);
char *imcdf_write_variable (int cdf_handle, struct IMCDFVariable *variable);
char *imcdf_write_time_stamps (int cdf_handle, struct IMCDFVariableTS *ts);
char *getINTERMAGNETTermsOfUse ();
int imcdf_is_vector_gm_data (enum IMCDFVariableType var_type, char *elem_rec);
int imcdf_is_scalar_gm_data (enum IMCDFVariableType var_type, char *elem_rec);
char *imcdf_make_filename (char *prefix, char *station_code, long long start_date,
			   enum IMCDFPubLevel pub_level,
                           double sample_period,
                           int force_lower_case, char *filename);

/* imcdf_low_level.c */
int imcdf_open (char *filename, enum IMCDFOpenType open_type, enum IMCDFCompressionType compress_type);
int imcdf_close (int cdf_handle);
int imcdf_add_global_attr_string (int cdf_handle, char *name, int entry_no, char *value);
int imcdf_add_global_attr_double (int cdf_handle, char *name, int entry_no, double value);
int imcdf_add_global_attr_tt2000 (int cdf_handle, char *name, int entry_no, long long value);
int imcdf_add_variable_attr_string (int cdf_handle, char *attr_name,
								    char *var_name, char *value);
int imcdf_add_variable_attr_double (int cdf_handle, char *attr_name,
								    char *var_name, double value);
int imcdf_add_variable_attr_tt2000 (int cdf_handle, char *attr_name,
								    char *var_name, long long value);
int imcdf_create_data_array (int cdf_handle, char *name, double *data,
                             int data_length);
int imcdf_create_time_stamp_array (int cdf_handle, char *name, long long *data,
                                   int data_length);
int imcdf_append_data_array (int cdf_handle, char *name, double *data,
                             int data_length);
int imcdf_append_time_stamp_array (int cdf_handle, char *name, long long *data,
                           int data_length);
int imcdf_get_global_attribute_string (int cdf_handle, char *name, int entry_no, char **value);
int imcdf_get_global_attribute_double (int cdf_handle, char *name, int entry_no, double *value);
int imcdf_get_global_attribute_tt2000 (int cdf_handle, char *name, int entry_no, long long *value);
int imcdf_get_variable_attribute_string (int cdf_handle, char *attr_name,
                                         char *var_name, char **value);
int imcdf_get_variable_attribute_double (int cdf_handle, char *attr_name,
                                         char *var_name, double *value);
double *imcdf_get_var_data (int cdf_handle, char *name, int *data_len);
long long *imcdf_get_var_time_stamps (int cdf_handle, char *name, int *data_len);
int imcdf_is_var_exist (int cdf_handle, char *name);
int imcdf_date_time_to_tt2000 (int year, int month, int day, int hour,
                               int min, int sec, long long *tt2000);
int imcdf_tt2000_to_date_time (long long tt2000,
                               int *year, int *month, int *day,
                               int *hour, int *min, int *sec);
long long imcdf_tt2000_inc (long long tt2000, int inc);
long long *imcdf_make_tt2000_array (int year, int month, int day, int hour, int min, int sec,
                                    int increment, int n_samples);
char *imcdf_tt2000_tostring (long long tt2000);
int imcdf_calc_samp_per_from_tt2000 (long long *tt2000_array);
CDFstatus imcdf_get_last_status_code ();
char *imcdf_status_code_tostring (CDFstatus status);
/* imcdf_utils.c */
enum IMCDFPubLevel imcdf_parse_pub_level_string (char *string);
char *imcdf_pub_level_code_tostring (enum IMCDFPubLevel code);
enum IMCDFStandardLevel imcdf_parse_standard_level_string (char *string);
char *imcdf_standard_level_code_tostring (enum IMCDFStandardLevel code);
enum IMCDFVariableType imcdf_parse_var_type_string (char *string);
char *imcdf_var_type_code_tostring (enum IMCDFVariableType code);
char *imcdf_get_var_name (enum IMCDFVariableType code, char element_code, char *var_name);
enum IMCDFPubLevel imcdf_dt_to_pub_level (char *dt);
void imcdf_print_global_attrs (struct IMCDFGlobalAttr *global_attrs);
void imcdf_print_variable (struct IMCDFVariable *variable, struct IMCDFVariableTS *time_stamps);


