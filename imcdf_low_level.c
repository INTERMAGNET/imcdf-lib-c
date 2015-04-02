/*****************************************************************************
 * imcdf_low_level.c - basic interactions with a CDF file - all the calls to
 *                     the CDF library are in this source code file - there is
 *                     no other linkage to the CDF library outside this file
 *
 * THE IMCDF ROUTINES SHOULD NOT HAVE DEPENDENCIES ON OTHER LIBRARY ROUTINES -
 * IT MUST BE POSSIBLE TO DISTRIBUTE THE IMCDF SOURCE CODE
 *
 * This code only used Extended Standard Interface functions
 *
 * access to an open CDF is controlled by a CDF handle, which is an index into
 * an array of CDFid elements
 *
 * Simon Flower, 19/12/2012
 * Updates to version 1.1 of ImagCDF. Simon Flower, 19/02/2015
 *****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdf.h"

#include "imcdf.h"

/* private global variables: */
/* an array of CDF ids - this allows for more than one CDF file to be kept open
 * at a time, though in reality it's unlikely that more than one will ever be
 * open simultaneously */
#define MAX_OPEN_CDF_FILES    10
static CDFid cdf_ids [MAX_OPEN_CDF_FILES];
static int cdf_index = -1;
/* the status of the last call to the CDF library */
CDFstatus cdf_status;

/* private forward declarations  */
static void initialise_cdf_ids ();
static int sanity_check_handles (int cdf_handle);
static long find_variable_attribute (int cdf_handle, char *name);

/** ------------------------------------------------------------------------
 *  --------------------- Opening and closing CDF files --------------------
 *  ------------------------------------------------------------------------*/

/*****************************************************************************
 * imcdf_open
 *
 * Description: open a CDF for reading or writing
 *
 * Input parameters: filename - the CDF file to open
 *                   open_type - how to open the file
 *                   compress_type - how to compress the file (only used
 *                                   on file that is being created)
 * Output parameters:
 * Returns: a handle (greater than or equal to zero) on success
 *          a negative number on failure
 *
 *****************************************************************************/
int imcdf_open (char *filename, enum IMCDFOpenType open_type,
                enum IMCDFCompressionType compress_type)
{
    long cparams [1];
    CDFid id;

    initialise_cdf_ids ();

    /* check there is space to open another file */
    if (cdf_index >= MAX_OPEN_CDF_FILES) return -1;

    /* open the file */
    switch (open_type)
    {
    case IMCDF_FORCE_CREATE:
        if (! access (filename, 0)) remove (filename);
        cdf_status = CDFcreateCDF (filename, &id);
    if (cdf_status < CDF_WARN) return -1;
        break;
    case IMCDF_CREATE:
        cdf_status = CDFcreateCDF (filename, &id);
        if (cdf_status < CDF_WARN) return -1;
        break;
    case IMCDF_OPEN:
        cdf_status = CDFopenCDF (filename, &id);
        if (cdf_status < CDF_WARN) return -1;
        compress_type = IMCDF_COMPRESS_NONE;
        break;
    default:
    return -1;
    }

    /* set the compression */
    switch (compress_type)
    {
    case IMCDF_COMPRESS_RLE:
        cparams [0] = RLE_OF_ZEROs;
        cdf_status = CDFsetCompression (id, RLE_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_HUFF:
        cparams [0] = OPTIMAL_ENCODING_TREES;
        cdf_status = CDFsetCompression (id, HUFF_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_AHUFF:
        cparams [0] = OPTIMAL_ENCODING_TREES;
        cdf_status = CDFsetCompression (id, AHUFF_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP1:
        cparams [0] = 1l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP2:
        cparams [0] = 2l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP3:
        cparams [0] = 3l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP4:
        cparams [0] = 4l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP5:
        cparams [0] = 5l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP6:
        cparams [0] = 6l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP7:
        cparams [0] = 7l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP8:
        cparams [0] = 8l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    case IMCDF_COMPRESS_GZIP9:
        cparams [0] = 9l;
        cdf_status = CDFsetCompression (id, GZIP_COMPRESSION, cparams);
        break;
    }
    if (cdf_status < CDF_WARN) return -1;

    /* insert the ID */
    cdf_ids [cdf_index] = id;
    return cdf_index ++;
}

/*****************************************************************************
 * imcdf_close
 *
 * Description: close a CDF - you MUST call this after writing to the CDF
 *              otherwise it will be corrupt
 *
 * Input parameters: cdf_handle - the CDF to close
 * Output parameters: none
 * Returns: 0 for success, -1 for failure
 *
 *****************************************************************************/
 int imcdf_close (int cdf_handle)

 {
    int count;

    if (sanity_check_handles (cdf_handle)) return -1;

    /* close the CDF */
    cdf_status = CDFcloseCDF (cdf_ids [cdf_handle]);
    if (cdf_status < CDF_WARN) return -1;

    /* sort out the array of CDF ids */
    for (count=cdf_handle +1; count<cdf_index; count++)
        cdf_ids [count -1] = cdf_ids [count];
    cdf_index --;

    return 0;
}


/** ------------------------------------------------------------------------
 *  ------------------------- Writing to CDF files -------------------------
 *  ------------------------------------------------------------------------*/

/****************************************************************************
 * imcdf_add_global_attr_string
 * imcdf_add_global_attr_double
 * imcdf_add_global_attr_tt2000
 *
 * Description: add a global attribute of the specified type to the CDF file
 *              and make an entry in it
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                      name - the attribute name (must be unique)
 *                   entry_no - the entry number required, 0..n_entries-1
 *                   value - the contents of the entry
 * Output parameters:
 * Returns: 0 for success, -1 for failure
 *
 ****************************************************************************/
int imcdf_add_global_attr_string (int cdf_handle, char *name, int entry_no, char *value)

{
    long attr_num;

    if (sanity_check_handles (cdf_handle)) return -1;

    if (value)
    {
        cdf_status = CDFcreateAttr (cdf_ids [cdf_handle], name, GLOBAL_SCOPE, &attr_num);
        if (cdf_status < CDF_WARN) return -1;
        cdf_status = CDFputAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no, CDF_CHAR, (long) strlen (value), value);
        if (cdf_status < CDF_WARN) return -1;
    }

    return 0;
}


int imcdf_add_global_attr_double (int cdf_handle, char *name, int entry_no, double value)

{
    long attr_num;
    double values [1];

    if (sanity_check_handles (cdf_handle)) return -1;

    cdf_status = CDFcreateAttr (cdf_ids [cdf_handle], name, GLOBAL_SCOPE, &attr_num);
    if (cdf_status < CDF_WARN) return -1;
    values [0] = value;
    cdf_status = CDFputAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no, CDF_DOUBLE, 1l, values);
    if (cdf_status < CDF_WARN) return -1;

    return 0;
}


int imcdf_add_global_attr_tt2000 (int cdf_handle, char *name, int entry_no, long long value)

{
    long attr_num;
    long long values [1];

    if (sanity_check_handles (cdf_handle)) return -1;

    cdf_status = CDFcreateAttr (cdf_ids [cdf_handle], name, GLOBAL_SCOPE, &attr_num);
    if (cdf_status < CDF_WARN) return -1;
    values [0] = value;
    cdf_status = CDFputAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no, CDF_TIME_TT2000, 1l, values);
    if (cdf_status < CDF_WARN) return -1;

    return 0;
}

/****************************************************************************
 * imcdf_add_variable_attr_string
 * imcdf_add_variable_attr_double
 * imcdf_add_variable_attr_tt2000
 *
 * Description: add a variable attribute of the specified type to the CDF file
 *              and make an entry in it
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                      attr_name - the attribute name (must be unique)
 *                   var_name - the name of the variable to add the entry to
 *                   value - the contents of the entry
 * Output parameters:
 * Returns: 0 for success, -1 for failure
 *
 ****************************************************************************/
int imcdf_add_variable_attr_string (int cdf_handle, char *attr_name,
                                    char *var_name, char *value)

{

    long attr_num, var_num;

    if (sanity_check_handles (cdf_handle)) return -1;

    attr_num = find_variable_attribute (cdf_handle, attr_name);
    if (attr_num < 0l) return -1;

    var_num = CDFgetVarNum (cdf_ids [ cdf_handle], var_name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) var_num;
        return -1;
    }

    cdf_status = CDFputAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num, CDF_CHAR, (long) strlen (value), value);
    if (cdf_status < CDF_WARN) return -1;

    return 0;
}


int imcdf_add_variable_attr_double (int cdf_handle, char *attr_name,
                                    char *var_name, double value)
{

    long attr_num, var_num;
    double values [1];

    if (sanity_check_handles (cdf_handle)) return -1;

    attr_num = find_variable_attribute (cdf_handle, attr_name);
    if (attr_num < 0l) return -1;

    var_num = CDFgetVarNum (cdf_ids [ cdf_handle], var_name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) var_num;
        return -1;
    }

    values [0] = value;
    cdf_status = CDFputAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num,
                                   CDF_DOUBLE, 1l, values);
    if (cdf_status < CDF_WARN) return -1;

    return 0;

}


int imcdf_add_variable_attr_tt2000 (int cdf_handle, char *attr_name,
                                    char *var_name, long long value)
{

    long attr_num, var_num;
    long long values [1];

    if (sanity_check_handles (cdf_handle)) return -1;

    attr_num = find_variable_attribute (cdf_handle, attr_name);
    if (attr_num < 0l) return -1;

    var_num = CDFgetVarNum (cdf_ids [ cdf_handle], var_name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) var_num;
        return -1;
    }

    values [0] = value;
    cdf_status = CDFputAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num,
                                   CDF_TIME_TT2000, 1l, values);
    if (cdf_status < CDF_WARN) return -1;

    return 0;

}

/****************************************************************************
 * imcdf_create_data_array
 * imcdf_create_time_stamp_array
 * imcdf_append_data_array
 * imcdf_append_time_stamp_array
 *
 * create a data array or a time stamp array in the CDF file
 * append data to a data array or a time stamp aray in the CDF file
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   name - the variable name
 *                   data - the data to write into the variable
 *                   data_length - the number of elements of data to write
 * Output parameters:
 * Returns: 0 for success, -1 for failure
 *
 ****************************************************************************/
int imcdf_create_data_array (int cdf_handle, char *name, double *data,
                             int data_length)

{
    int count;
    long var_num, dim_size [1], dim_var [1];

    if (sanity_check_handles (cdf_handle)) return -1;

    dim_size [0] = 1;
    dim_var [0] = VARY;
    cdf_status = CDFcreatezVar (cdf_ids [cdf_handle], name, CDF_DOUBLE,
                1l, 0l, dim_size, VARY, dim_var, &var_num);
    if ((cdf_status < CDF_WARN)&&(cdf_status != VAR_EXISTS)) return -1;

    return imcdf_append_data_array (cdf_handle, name, data, data_length);
}


int imcdf_create_time_stamp_array (int cdf_handle, char *name, long long *data,
                                   int data_length)

{
    int count;
    long var_num, dim_size [1], dim_var [1];

    if (sanity_check_handles (cdf_handle)) return -1;

    dim_size [0] = 1;
    dim_var [0] = VARY;
    cdf_status = CDFcreatezVar (cdf_ids [cdf_handle], name, CDF_TIME_TT2000,
                1l, 0l, dim_size, VARY, dim_var, &var_num);
    if ((cdf_status < CDF_WARN)&&(cdf_status != VAR_EXISTS)) return -1;

    return imcdf_append_time_stamp_array (cdf_handle, name, data, data_length);
}


int imcdf_append_data_array (int cdf_handle, char *name, double *data,
                             int data_length)

{
    long var_num, n_recs, count;

    if (sanity_check_handles (cdf_handle)) return -1;

    var_num = CDFgetVarNum (cdf_ids [cdf_handle], name);
    if (var_num < 0)
    {
        cdf_status = var_num;
        return -1;
    }

    cdf_status = CDFgetzVarMaxWrittenRecNum (cdf_ids [cdf_handle], var_num, &n_recs);
    /*cdf_status = CDFgetzVarNumRecsWritten (cdf_ids [cdf_handle], var_num, &n_recs);*/
    if (cdf_status != CDF_OK) return -1;

    //if (n_recs < 0) n_recs = 0;
    ++n_recs; // n_recs=-1 for new data, =max existing record number for existing data; either way next record is +1

    for (count=0; count<data_length; count++)
    {
        cdf_status = CDFputzVarRecordData (cdf_ids [cdf_handle], var_num, count + n_recs, data + count);
        if (cdf_status < CDF_WARN) return -1;
    }

    return 0;
}


int imcdf_append_time_stamp_array (int cdf_handle, char *name, long long *data,
                                   int data_length)

{
    long var_num, n_recs, count;

    if (sanity_check_handles (cdf_handle)) return -1;

    var_num = CDFgetVarNum (cdf_ids [cdf_handle], name);
    if (var_num < 0)
    {
        cdf_status = var_num;
        return -1;
    }

    cdf_status = CDFgetzVarMaxWrittenRecNum (cdf_ids [cdf_handle], var_num, &n_recs);
    if (cdf_status != CDF_OK) return -1;
    //if (n_recs < 0) n_recs = 0;
    ++n_recs; // n_recs=-1 for new data, =max existing record number for existing data; either way next record is +1

    for (count=0; count<data_length; count++)
    {
        cdf_status = CDFputzVarRecordData (cdf_ids [cdf_handle], var_num, count + n_recs, data + count);// mod PGC +1
        if (cdf_status < CDF_WARN) return -1;
    }

    return 0;
}

/** ------------------------------------------------------------------------
 *  ----------------------- Reading from CDF files -------------------------
 *  ------------------------------------------------------------------------*/

/****************************************************************************
 * imcdf_get_global_attribute_string
 * imcdf_get_global_attribute_double
 * imcdf_get_global_attribute_tt2000
 *
 * Description: get the contents of a global attribute
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   name - the name of the attribute
 *                   entry_no - the entry number required, 0..n_entries-1
 * Output parameters: value - the value of the attribute - for strings
 *                            this will point to dynamically allocated memory
 * Returns: 0 for success, -1 for failure
 *
 ****************************************************************************/
int imcdf_get_global_attribute_string (int cdf_handle, char *name, int entry_no, char **value)
{
    long attr_num, data_type, num_elements;

    if (sanity_check_handles (cdf_handle)) return -1;

    attr_num = CDFattrNum (cdf_ids [cdf_handle], name);
    if (attr_num < 0)
    {
        cdf_status = attr_num;
        return -1;
    }

    cdf_status = CDFinquireAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no,
                                       &data_type, &num_elements);
    if (cdf_status < 0) return -1;
    if (data_type != CDF_CHAR) return -1;

    *value = malloc (num_elements +1);
    if (! *value)
    {
        cdf_status = BAD_MALLOC;
        return -1;
    }

    cdf_status = CDFgetAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no, *value);
    if (cdf_status < 0) return -1;
    *((*value) + num_elements) = '\0';

    return 0;
}


int imcdf_get_global_attribute_double (int cdf_handle, char *name, int entry_no, double *value)
{
    long attr_num, data_type, num_elements;

    if (sanity_check_handles (cdf_handle)) return -1;

    attr_num = CDFattrNum (cdf_ids [cdf_handle], name);
    if (attr_num < 0)
    {
        cdf_status = attr_num;
        return -1;
    }

    cdf_status = CDFinquireAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no,
                                       &data_type, &num_elements);
    if (cdf_status < 0) return -1;
    if (data_type != CDF_DOUBLE) return -1;

    cdf_status = CDFgetAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no, value);
    if (cdf_status < 0) return -1;

    return 0;
}


int imcdf_get_global_attribute_tt2000 (int cdf_handle, char *name, int entry_no, long long *value)
{
    long attr_num, data_type, num_elements;

    if (sanity_check_handles (cdf_handle)) return -1;

    attr_num = CDFattrNum (cdf_ids [cdf_handle], name);
    if (attr_num < 0)
    {
        cdf_status = attr_num;
        return -1;
    }

    cdf_status = CDFinquireAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no,
                                       &data_type, &num_elements);
    if (cdf_status < 0) return -1;
    if (data_type != CDF_TIME_TT2000) return -1;

    cdf_status = CDFgetAttrgEntry (cdf_ids [cdf_handle], attr_num, entry_no, value);
    if (cdf_status < 0) return -1;

    return 0;
}

/****************************************************************************
 * imcdf_get_variable_attribute_string
 * imcdf_get_variable_attribute_double
 * imcdf_get_variable_attribute_tt2000
 *
 * Description: get the contents of a variable attribute
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                      attr_name - the attribute name
 *                   var_name - the name of the variable
 * Output parameters: value - the value of the attribute - for strings
 *                            this will point to dynamically allocated memory
 * Returns: 0 for success, -1 for failure
 *
 ****************************************************************************/
int imcdf_get_variable_attribute_string (int cdf_handle, char *attr_name,
                                         char *var_name, char **value)

{
    long var_num, attr_num, data_type, num_elements;

    if (sanity_check_handles (cdf_handle)) return -1;

    var_num = CDFgetVarNum (cdf_ids [ cdf_handle], var_name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) cdf_status;
        return -1;
    }

    attr_num = CDFattrNum (cdf_ids [cdf_handle], attr_name);
    if (attr_num < 0)
    {
        cdf_status = attr_num;
        return -1;
    }

    cdf_status = CDFinquireAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num,
                                       &data_type, &num_elements);
    if (cdf_status < 0) return -1;
    if (data_type != CDF_CHAR) return -1;

    *value = malloc (num_elements +1);
    if (! *value)
    {
        cdf_status = BAD_MALLOC;
        return -1;
    }

    cdf_status = CDFgetAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num, *value);
    if (cdf_status < 0) return -1;
    *((*value) + num_elements) = '\0';

    return 0;
}


int imcdf_get_variable_attribute_double (int cdf_handle, char *attr_name,
                                         char *var_name, double *value)

{
    long var_num, attr_num, data_type, num_elements;

    if (sanity_check_handles (cdf_handle)) return -1;

    var_num = CDFgetVarNum (cdf_ids [ cdf_handle], var_name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) cdf_status;
        return -1;
    }

    attr_num = CDFattrNum (cdf_ids [cdf_handle], attr_name);
    if (attr_num < 0)
    {
        cdf_status = attr_num;
        return -1;
    }

    cdf_status = CDFinquireAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num,
                                       &data_type, &num_elements);
    if (cdf_status < 0) return -1;
    if (data_type != CDF_DOUBLE) return -1;

    cdf_status = CDFgetAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num, value);
    if (cdf_status < 0) return -1;

    return 0;
}


int imcdf_get_variable_attribute_tt2000 (int cdf_handle, char *attr_name,
                                         char *var_name, long long *value)

{
    long var_num, attr_num, data_type, num_elements;

    if (sanity_check_handles (cdf_handle)) return -1;

    var_num = CDFgetVarNum (cdf_ids [ cdf_handle], var_name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) cdf_status;
        return -1;
    }

    attr_num = CDFattrNum (cdf_ids [cdf_handle], attr_name);
    if (attr_num < 0)
    {
        cdf_status = attr_num;
        return -1;
    }

    cdf_status = CDFinquireAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num,
                                       &data_type, &num_elements);
    if (cdf_status < 0) return -1;
    if (data_type != CDF_TIME_TT2000) return -1;

    cdf_status = CDFgetAttrzEntry (cdf_ids [cdf_handle], attr_num, var_num, value);
    if (cdf_status < 0) return -1;

    return 0;
}

/***************************************************************************
 * imcdf_get_var_data
 * imcdf_get_var_time_stamp
 *
 * Description: get data from a data variable or a timestamp variable
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   name - the name of the variable
 * Output parameters: data_len - the length of the retrieved data
 * Returns: the data in a newly allocated memory space or NULL if there
 *          is a failure
 *
 ****************************************************************************/
double *imcdf_get_var_data (int cdf_handle, char *var_name, int *data_len)
{

    long var_num, data_type, num_elements, num_dims, dim_sizes [CDF_MAX_DIMS];
    long rec_variance, dim_variance [CDF_MAX_DIMS], count;
    double *data;

    if (sanity_check_handles (cdf_handle)) return 0;

    var_num = CDFgetVarNum (cdf_ids [cdf_handle], var_name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) cdf_status;
        return 0;
    }

    cdf_status = CDFinquirezVar (cdf_ids [cdf_handle], var_num, var_name,
                                 &data_type, &num_elements, &num_dims, dim_sizes,
                                 &rec_variance, dim_variance);
    if (cdf_status < 0) return 0;
    if (data_type != CDF_DOUBLE) return 0;
    if (num_dims != 0) return 0;

    cdf_status = CDFgetzVarNumRecsWritten (cdf_ids [cdf_handle], var_num, data_len);
    if (cdf_status != CDF_OK) return 0;

    data = malloc (*data_len * sizeof (double));
    if (! data)
    {
        cdf_status = BAD_MALLOC;
        return 0;
    }

    for (count=0; count<*data_len; count++)
    {
        cdf_status = CDFgetzVarRecordData (cdf_ids [cdf_handle], var_num, count, data + count);
        if (cdf_status < 0)
        {
            free (data);
            return 0;
        }
    }

    return data;
}


long long *imcdf_get_var_time_stamps (int cdf_handle, char *var_name, int *data_len)
{

    long var_num, data_type, num_elements, num_dims, dim_sizes [CDF_MAX_DIMS];
    long rec_variance, dim_variance [CDF_MAX_DIMS], count;
    long long *data;

    if (sanity_check_handles (cdf_handle)) return 0;

    var_num = CDFgetVarNum (cdf_ids [cdf_handle], var_name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) cdf_status;
        return 0;
    }
	char filler[1024];
//    cdf_status = CDFinquirezVar (cdf_ids [cdf_handle], var_num, var_name,
//                                 &data_type, &num_elements, &num_dims, dim_sizes,
//                                 &rec_variance, dim_variance);
    cdf_status = CDFinquirezVar (cdf_ids [cdf_handle], var_num, filler,
                                 &data_type, &num_elements, &num_dims, dim_sizes,
                                 &rec_variance, dim_variance);
    if (cdf_status < 0) return 0;
    if (data_type != CDF_TIME_TT2000) return 0;
    if (num_dims != 0) return 0;

    cdf_status = CDFgetzVarNumRecsWritten (cdf_ids [cdf_handle], var_num, data_len);
    if (cdf_status != CDF_OK) return 0;

    data = malloc (*data_len * sizeof (long long));
    if (! data)
    {
        cdf_status = BAD_MALLOC;
        return 0;
    }

    for (count=0; count<*data_len; count++)
    {
        cdf_status = CDFgetzVarRecordData (cdf_ids [cdf_handle], var_num, count, data + count);
        if (cdf_status < 0)
        {
            free (data);
            return 0;
        }
    }

    return data;
}

/***************************************************************************
 * imcdf_is_var_exist
 *
 * Description: test if the given variable exists
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   name - the name of the variable
 * Output parameters:
 * Returns: 0 if variable is found, -1 otherwise
 *
 ****************************************************************************/
int imcdf_is_var_exist (int cdf_handle, char *name)

{

    long var_num;

    var_num = CDFgetVarNum (cdf_ids [ cdf_handle], name);
    if (var_num < 0l)
    {
        cdf_status = (CDFstatus) var_num;
        return -1;
    }
    return 0;
}


/** ------------------------------------------------------------------------
 *  ---------------------- TT2000 data manipulation ------------------------
 *  ------------------------------------------------------------------------*/

/****************************************************************************
 * imcdf_date_time_to_tt2000
 * imcdf_tt2000_to_date_time
 *
 * Description: convert date/time to / from TT2000
 *
 * Input parameters: year, month, day - the date - month and day start at 1
 *                   hour, min, sec - the time (all 0 based)
 * Output parameters: tt2000 - the output tt2000 value
 * Returns: 0 if conversion was completed OK, -1 otherwise
 *
 ****************************************************************************/
int imcdf_date_time_to_tt2000 (int year, int month, int day, int hour,
                               int min, int sec, long long *tt2000)
{
    *tt2000 = CDF_TT2000_from_UTC_parts ((double) year, (double) month, (double) day,
                                         (double) hour, (double) min, (double) sec,
                                         0.0, 0.0, 0.0);
    if (*tt2000 == ILLEGAL_TT2000_VALUE)
    {
        cdf_status = BAD_ARGUMENT;
        return -1;
    }
    cdf_status = CDF_OK;
    return 0;
}


int imcdf_tt2000_to_date_time (long long tt2000,
                               int *year, int *month, int *day,
                               int *hour, int *min, int *sec)
{
    double d_year, d_month, d_day, d_hour, d_min, d_sec;

    CDF_TT2000_to_UTC_parts (tt2000, &d_year, &d_month, &d_day,
                             &d_hour, &d_min, &d_sec, TT2000NULL);
    *year = (int) d_year;
    *month = (int) d_month;
    *day = (int) d_day;
    *hour = (int) d_hour;
    *min = (int) d_min;
    *sec = (int) (d_sec + 0.5);
    cdf_status = CDF_OK;
    return 0;
}


/****************************************************************************
 * imcdf_tt2000_inc
 *
 * Description: increment (or decrement) a TT2000 date/time
 *
 * Input parameters: tt2000 - the date / time to increment or decrement
 *                   inc - the amount to increment or decrement by (in seconds)
 * Output parameters:
 * Returns: the new TT2000 value
 *
 ****************************************************************************/
long long imcdf_tt2000_inc (long long tt2000, int inc)

{
    return tt2000 + ((long long) inc * 1000000000ll);
}

/****************************************************************************
 * imcdf_make_tt2000_array
 *
 * Description: create an array of TT2000 time stamps
 *
 * Input parameters: year, month, day - the date - month and day start at 1
 *                   hour, min, sec - the time (all 0 based)
 *                   increment - the time increment between samples, in seconds
 *                   n_samples - the number of samples
 * Output parameters:
 * Returns: an array of TT2000 time stamps, allocated dynamically (so will need
 *          to be freed when done with) OR null if there was a problem.
 *          If this routine returns null, call imcdf_get_last_status_code()
 *          to see what happened - if it returns CDF_OK there was a memory
 *          allocation fault.
 *
 ****************************************************************************/
long long *imcdf_make_tt2000_array (int year, int month, int day, int hour, int min, int sec,
                                    int increment, int n_samples)

{
    int count;
    long long *tt2000_array, tt2000;


    tt2000_array = malloc (sizeof (long long) * n_samples);
    if (! tt2000_array) return 0;

    if (imcdf_date_time_to_tt2000 (year, month, day, hour, min, sec, &tt2000))
    {
        free (tt2000_array);
        return 0;
    }

    for (count=0; count<n_samples; count++)
    {
        *(tt2000_array + count) = tt2000;
        tt2000 = imcdf_tt2000_inc (tt2000, increment);
    }

    return tt2000_array;
}


/****************************************************************************
 * imcdf_calc_samp_per_from_tt2000
 *
 * Description: calculate the sample period of a TT2000 array
 *
 * Input parameters: tt2000_array - the array to calculate the period for
 *                                  minimum length 2 samples
 * Output parameters: none
 * Returns: the sample period, in seconds
 ****************************************************************************/
int imcdf_calc_samp_per_from_tt2000 (long long *tt2000_array)

{

    long diff;

    diff = (long) (*(tt2000_array +1) - *tt2000_array);
    return (int) (diff / 1000000000l);

}


/****************************************************************************
 * imcdf_tt2000_tostring
 *
 * Description: format a TT2000 object as a string
 *
 * Input parameters: tt2000 - the date/time object to format
 * Output parameters: none
 * Returns: an ISO format date/time string
 *
 * The return points to a static buffer which will be overwritten on each
 * call to this function
 ****************************************************************************/
char *imcdf_tt2000_tostring (long long tt2000)

{

  static char buffer [30];

  CDF_TT2000_to_UTC_string (tt2000, buffer, 3);
  buffer [19] = '\0';
  return buffer;

}


/** ------------------------------------------------------------------------
 *  --------------------------- Error notification -------------------------
 *  ------------------------------------------------------------------------*/

/*****************************************************************************
 * imcdf_get_last_status_code
 *
 * Description: get the status code (which may have been successful) from
 *              the last call to the CDF library
 *
 * Input parameters:
 * Output parameters:
 * Returns: the status code
 *
 *****************************************************************************/
CDFstatus imcdf_get_last_status_code ()

{
    return cdf_status;
}

/*****************************************************************************
 * imcdf_status_code_tostring
 *
 * Description: decode a CDF status code to something that can be displayed to
 *              a user
 *
 * Input parameters: status - the status code
 * Output parameters:
 * Returns: the status code (in a static buffer - a call to this function
 *          will change the contents returned from any previous calls)
 *
 *****************************************************************************/
char *imcdf_status_code_tostring (CDFstatus status)

{
    char cdf_msg[CDF_STATUSTEXT_LEN+1];

    static char message[CDF_STATUSTEXT_LEN+30];

    if (status < CDF_WARN)
    {
        CDFgetStatusText (status, cdf_msg);
        sprintf (message, "Error: %s\n", cdf_msg);
    }
    else if (status < CDF_OK)
    {
        CDFgetStatusText (status, cdf_msg);
        sprintf (message, "Warning: %s\n", cdf_msg);
    }
    else if (status > CDF_OK)
    {
        CDFgetStatusText (status, cdf_msg);
        sprintf (message, "Information: %s\n", cdf_msg);
    }
    else
    {
        strcpy (message, "Success");
    }

    return message;
}


/** ------------------------------------------------------------------------
 *  ---------------------------- Private code ------------------------------
 *  ------------------------------------------------------------------------*/

/* initialise the array of CDF ids */
static void initialise_cdf_ids ()
{
    int count;

    if (cdf_index < 0)
    {
        for (count=0; count<MAX_OPEN_CDF_FILES; count++)
            cdf_ids [count] = (CDFid) 0;
        cdf_index = 0;

        cdf_status = CDF_OK;
    }
}

/* code used at the start of each function to check sanity of the
 * state of the cdf handles */
static int sanity_check_handles (int cdf_handle)
{
    initialise_cdf_ids ();
    if (cdf_handle < 0) return -1;
    if (cdf_handle >= cdf_index) return -1;
    return 0;
}

/* find a variable attribute - if it doesn't exist create it */
static long find_variable_attribute (int cdf_handle, char *name)
{
    long attr_num;

    attr_num = CDFgetAttrNum (cdf_ids [cdf_handle], name);
    if (attr_num < 0)
    {
        cdf_status = CDFcreateAttr (cdf_ids [cdf_handle], name, VARIABLE_SCOPE, &attr_num);
        if (cdf_status < CDF_WARN) return -1l;
    }
    return attr_num;
}

