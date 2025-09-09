/* test the IMAG CDF routines using synthetic data */

#include <time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "imcdf.h"

#define N_VARS 5
#define N_SAMPLES 1440

static char cdf_filename [100] = "";

void handle_error (char *err_msg);


int main ()

{
  int count, count2, cdf_handle;
  char field_name [N_VARS] [40], var_name [40], *elem_ptr, elem_rec [10];
  double data [N_VARS] [N_SAMPLES], amp_scale, amp_offest, freq_mult, scale;
  struct IMCDFGlobalAttr global_attrs;
  struct IMCDFVariable variable [N_VARS], var;
  struct IMCDFVariableTS time_stamps;

  /* create fake data signals */
  for (count=0; count<N_VARS; count++)
  {
    switch (count)
    {
    case 0:  amp_scale = 10.0; amp_offest = 20000.0; break;
    case 1:  amp_scale =  1.0; amp_offest =   -20.0; break;
    case 2:  amp_scale = 20.0; amp_offest = 30000.0; break;
    case 3:  amp_scale = 35.0; amp_offest = 50000.0; break;
    default: amp_scale =  2.0; amp_offest =    20.0; break;
    }
    freq_mult = (double) (count + 1);
    scale = (M_PI * 2.0 * freq_mult) / (double) N_SAMPLES;
    for (count2=0; count2<N_SAMPLES; count2++)
      data [count] [count2] = amp_offest + (amp_scale * sin ((double) count2 * scale));
  }

  /* create fake global attributes */
  global_attrs.format_description = "";
  global_attrs.format_version = "";
  global_attrs.title = "";
  global_attrs.iaga_code = "AFO";
  global_attrs.elements_recorded = "HDZS";
  global_attrs.pub_level = IMCDF_PUBLEVEL_1;
  imcdf_date_time_to_tt2000 (2000, 1, 1, 0, 0, 0, &global_attrs.pub_date); 
  global_attrs.observatory_name = "A Fake Observatory";
  global_attrs.latitude = 0.0;
  global_attrs.longitude = 0.0;
  global_attrs.elevation = 0.0;  
  global_attrs.institution = "INTERMANGET";
  global_attrs.vector_sens_orient = "ABC";
  global_attrs.standard_level = IMCDF_STANDLEVEL_NONE;
  global_attrs.standard_name = 0;
  global_attrs.standard_version = 0;
  global_attrs.partial_stand_desc = 0;
  global_attrs.source = "INTERMAGNET";  
  global_attrs.terms_of_use = "";
  global_attrs.unique_identifier = 0;
  global_attrs.parent_identifiers = 0;
  global_attrs.n_parent_identifiers = 0;
  global_attrs.reference_links = 0;
  global_attrs.n_reference_links = 0;

  /* create fake variables */
  for (count=0; count<N_VARS; count++)
  {
    switch (count)
    {
    case 0:
      variable[count].var_type = IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT;
      sprintf (field_name [count], "Geomagnetic Field Element %c", global_attrs.elements_recorded [count]);
      variable[count].valid_min = -80000.0;
      variable[count].valid_max = 80000.0;
      variable[count].units = "nT";
      variable[count].elem_rec[0] = global_attrs.elements_recorded [count];
      variable[count].elem_rec[1] = '\0';
      break;
    case 1:
      variable[count].var_type = IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT;
      sprintf (field_name [count], "Geomagnetic Field Element %c", global_attrs.elements_recorded [count]);
      variable[count].valid_min = -360.0;
      variable[count].valid_max = 360.0;
      variable[count].units = "Degrees of arc";
      variable[count].elem_rec[0] = global_attrs.elements_recorded [count];
      variable[count].elem_rec[1] = '\0';
      break;
    case 2:
      variable[count].var_type = IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT;
      sprintf (field_name [count], "Geomagnetic Field Element %c", global_attrs.elements_recorded [count]);
      variable[count].valid_min = -80000.0;
      variable[count].valid_max = 80000.0;
      variable[count].units = "nT";
      variable[count].elem_rec[0] = global_attrs.elements_recorded [count];
      variable[count].elem_rec[1] = '\0';
      break;
    case 3:
      variable[count].var_type = IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT;
      sprintf (field_name [count], "Geomagnetic Field Element %c", global_attrs.elements_recorded [count]);
      variable[count].valid_min = 0.0;
      variable[count].valid_max = 80000.0;
      variable[count].units = "nT";
      variable[count].elem_rec[0] = global_attrs.elements_recorded [count];
      variable[count].elem_rec[1] = '\0';
      break;
    default:
      variable[count].var_type = IMCDF_VARTYPE_TEMPERATURE;
      sprintf (field_name [count], "Temperature %d", count -3);
      variable[count].valid_min = -100.0;
      variable[count].valid_max = 100.0;
      variable[count].units = "Celcius";
      sprintf (variable[count].elem_rec, "%d", count -3);
      break;
    }
    variable[count].field_nam = field_name [count];
    variable[count].fill_val = IMCDF_MISSING_DATA_VALUE;
    variable[count].data = data [count];
    variable[count].data_len = N_SAMPLES;
    variable[count].depend_0 = DATA_TIMES_VAR_NAME;
  }

  /* create time stamps */
  time_stamps.time_stamps = imcdf_make_tt2000_array (1980, 1, 1, 0, 0, 0, 60, N_SAMPLES);
  time_stamps.data_len = N_SAMPLES;
  time_stamps.var_name = DATA_TIMES_VAR_NAME;
  
  /* create a filename for the CDF file */
  imcdf_make_filename ("", global_attrs.iaga_code, time_stamps.time_stamps[0],
                       global_attrs.pub_level, IMCDF_INT_MINUTE,
                       IMCDF_INT_DAILY, true, cdf_filename);

  /* write the CDF file */
  handle_error (imcdf_open2 (cdf_filename, IMCDF_FORCE_CREATE, IMCDF_COMPRESS_GZIP5, &cdf_handle));
  handle_error (imcdf_write_global_attrs (cdf_handle, &global_attrs));
  for (count=0; count<N_VARS; count++)
    handle_error (imcdf_write_variable (cdf_handle, &variable[count], true));
  handle_error (imcdf_write_time_stamps (cdf_handle, &time_stamps));
  handle_error (imcdf_close2 (cdf_handle));

  /* read and print the CDF file */
  handle_error (imcdf_open2 (cdf_filename, IMCDF_OPEN, IMCDF_COMPRESS_NONE, &cdf_handle));
  handle_error (imcdf_read_global_attrs (cdf_handle, &global_attrs));
  imcdf_print_global_attrs (&global_attrs);
  /* read and print geomagnetic variables */
  handle_error (imcdf_read_time_stamps (cdf_handle, DATA_TIMES_VAR_NAME, &time_stamps));
  for (elem_ptr=global_attrs.elements_recorded; *elem_ptr; elem_ptr ++)
  {
    elem_rec [0] = *elem_ptr;
    elem_rec [1] = '\0';
    handle_error (imcdf_read_variable (cdf_handle, IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT, elem_rec, &var));
    imcdf_print_variable (&var, &time_stamps);
    imcdf_free_variable (&var);
  }
  /* read and print temperature variables */
  for (count=1; 1; count++)
  {
    sprintf (elem_rec, "%d", count);
    if (imcdf_read_variable (cdf_handle, IMCDF_VARTYPE_TEMPERATURE, elem_rec, &var)) break;
    sprintf (var_name, TEMPERATURE_TIME_STAMPS_VAR_NAME_BASE, elem_rec);
    imcdf_print_variable (&var, &time_stamps);
    imcdf_free_variable (&var);
  }
  /* tidy up */
  imcdf_free_time_stamps (&time_stamps);
  imcdf_free_global_attrs (&global_attrs);
  handle_error (imcdf_close2 (cdf_handle));

  exit (0);

}


void handle_error (char *err_msg)
{
  if (err_msg)
  {
    fprintf (stderr, "Error with CDF file [%s]: %s\n", cdf_filename, err_msg);
    exit (1);
  }
}

