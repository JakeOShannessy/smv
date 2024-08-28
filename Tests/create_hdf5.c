#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "hdf5_hl.h"
#include <hdf5.h>

herr_t add_dataset(hid_t file_id) {
  herr_t status = 0;
  // Create the dataspace for the dataset.
  hsize_t dims[2];
  dims[0] = 4;
  dims[1] = 6;

  hid_t dataspace_id = H5Screate_simple(2, dims, NULL);

  // Create the dataset.
  hid_t dataset_id = H5Dcreate(file_id, "/dset", H5T_STD_I32BE, dataspace_id,
                               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  int dset_data[24];
  for(size_t i = 0; i < 4; i++) {
    for(size_t j = 0; j < 6; j++) {
      dset_data[i * 6 + j] = i * 4 + j + 23;
    }
  }
  status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    dset_data);
  if(status < 0) goto fail;
  status = H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                   dset_data);
  if(status < 0) goto fail;

  // Close the dataset and dataspace
fail:
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
  return status;
}

herr_t add_time_bounds(hid_t file_id) {
  herr_t status;

  // Initialize the attribute data.
  float attr_data[2];
  attr_data[0] = -5.0;
  attr_data[1] = 10.0;

  // Create the data space for the attribute.
  hsize_t dims = 2;
  hid_t dataspace_id = H5Screate_simple(1, &dims, NULL);

  // Create a dataset attribute.
  hid_t attribute_id = H5Acreate2(file_id, "TimeBounds", H5T_IEEE_F32LE,
                                  dataspace_id, H5P_DEFAULT, H5P_DEFAULT);

  // Write the attribute data.
  status = H5Awrite(attribute_id, H5T_IEEE_F32LE, attr_data);

  // Close the attribute.
  H5Aclose(attribute_id);

  // Close the dataspace.
  H5Sclose(dataspace_id);
  return status;
}

herr_t add_string_attribute(hid_t file_id, const char *attr_name,
                            const char *attr_value) {
  herr_t status = 0;

  // Initialize the attribute data.
  const char *attr_data[1];
  attr_data[0] = attr_value;

  hid_t attr_space = H5Screate(H5S_SCALAR);
  hid_t attr_type = H5Tcopy(H5T_C_S1);
  status = H5Tset_size(attr_type, H5T_VARIABLE);
  if(status < 0) goto fail_set_size;
  // Create a dataset attribute.
  hid_t attribute_id = H5Acreate2(file_id, attr_name, attr_type, attr_space,
                                  H5P_DEFAULT, H5P_DEFAULT);

  // Write the attribute data.
  status = H5Awrite(attribute_id, attr_type, attr_data);
  if(status < 0) goto fail;
fail:
  H5Tclose(attr_type);
  // Close the attribute.
  H5Aclose(attribute_id);
fail_set_size:
  // Close the dataspace.
  H5Sclose(attr_space);
  return status;
}

herr_t add_float(hid_t file_id, const char *attr_name, float attr_value) {
  herr_t status = 0;
  float attr_data[1];
  attr_data[0] = attr_value;
  hid_t attr_space = H5Screate(H5S_SCALAR);
  if(status < 0) goto fail_set_size;
  // Create a dataset attribute.
  hid_t attribute_id = H5Acreate2(file_id, attr_name, H5T_IEEE_F32LE,
                                  attr_space, H5P_DEFAULT, H5P_DEFAULT);
  // Write the attribute data.
  status = H5Awrite(attribute_id, H5T_NATIVE_FLOAT, &attr_value);
  if(status < 0) goto fail;
fail:
  // Close the attribute.
  H5Aclose(attribute_id);
fail_set_size:
  // Close the dataspace.
  H5Sclose(attr_space);
  return status;
}

herr_t add_double(hid_t file_id, const char *attr_name, double attr_value) {
  herr_t status = 0;
  double attr_data[1];
  attr_data[0] = attr_value;
  hid_t attr_space = H5Screate(H5S_SCALAR);
  if(status < 0) goto fail_set_size;
  // Create a dataset attribute.
  hid_t attribute_id = H5Acreate2(file_id, attr_name, H5T_IEEE_F64LE,
                                  attr_space, H5P_DEFAULT, H5P_DEFAULT);
  // Write the attribute data.
  status = H5Awrite(attribute_id, H5T_NATIVE_DOUBLE, &attr_value);
  if(status < 0) goto fail;
fail:
  // Close the attribute.
  H5Aclose(attribute_id);
fail_set_size:
  // Close the dataspace.
  H5Sclose(attr_space);
  return status;
}

herr_t add_int(hid_t file_id, const char *attr_name, int attr_value) {
  herr_t status = 0;
  int attr_data[1];
  attr_data[0] = attr_value;
  hid_t attr_space = H5Screate(H5S_SCALAR);
  if(status < 0) goto fail_set_size;
  // Create a dataset attribute.
  hid_t attribute_id = H5Acreate2(file_id, attr_name, H5T_STD_I32BE, attr_space,
                                  H5P_DEFAULT, H5P_DEFAULT);
  // Write the attribute data.
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, &attr_value);
  if(status < 0) goto fail;
fail:
  // Close the attribute.
  H5Aclose(attribute_id);
fail_set_size:
  // Close the dataspace.
  H5Sclose(attr_space);
  return status;
}

herr_t set_units(hid_t dataset_id, const char *units) {
  return add_string_attribute(dataset_id, "Units", units);
}

herr_t set_quantity(hid_t dataset_id, const char *units) {
  return add_string_attribute(dataset_id, "Quantity", units);
}

herr_t set_short_name(hid_t dataset_id, const char *units) {
  return add_string_attribute(dataset_id, "ShortName", units);
}

herr_t set_bounds(hid_t dataset_id, int ijk[6]) {
  hid_t attribute_id, dataspace_id; // identifiers
  hsize_t dims;
  herr_t status;

  // Create the data space for the attribute.
  dims = 6;
  dataspace_id = H5Screate_simple(1, &dims, NULL);

  // Create a dataset attribute.
  attribute_id = H5Acreate2(dataset_id, "Bounds", H5T_STD_I32BE, dataspace_id,
                            H5P_DEFAULT, H5P_DEFAULT);

  // Write the attribute data.
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, ijk);

  // Close the attribute.
  status = H5Aclose(attribute_id);

  // Close the dataspace.
  status = H5Sclose(dataspace_id);
}

herr_t add_slice_dataset(hid_t file_id) {
  herr_t status = 0;
  // Create the dataspace for the dataset.
  hsize_t dims[3];
  dims[0] = 4;
  dims[1] = 6;
  dims[2] = 6;

  hid_t dataspace_id = H5Screate_simple(3, dims, NULL);

  // Create the dataset.
  hid_t dataset_id = H5Dcreate(file_id, "/slice", H5T_IEEE_F64LE, dataspace_id,
                               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  double dset_data[144];
  for(size_t i = 0; i < 4; i++) {
    for(size_t j = 0; j < 6; j++) {
      for(size_t k = 0; k < 6; k++) {
        size_t index = i * 6 * 6 + j * 6 + k;
        dset_data[index] = i * 4 + j + 23;
      }
    }
  }
  status = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                    H5P_DEFAULT, dset_data);
  if(status < 0) goto fail;
  status = H5Dread(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                   dset_data);
  if(status < 0) goto fail;

  status = set_quantity(dataset_id, "VELOCITY");
  status = set_short_name(dataset_id, "vel");
  status = set_units(dataset_id, "m/s");
  status = add_double(dataset_id, "Time", 15.0);
  status = add_int(dataset_id, "Mesh", 2);
  int ijk[6] = {2, 6, 6, 12, 6, 12};
  status = set_bounds(dataset_id, ijk);
  if(status < 0) goto fail;

  // Close the dataset and dataspace
fail:
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
  return status;
}

typedef struct _blockagedata {
  int ijk[6];
  char *name;
  float xmin, xmax, ymin, ymax, zmin, zmax;
} blockagedata;

herr_t add_obsts_list(hid_t file_id) {
  herr_t status = 0;
  // Create the dataspace for the dataset.
  hsize_t dims[3];
  dims[0] = 4;
  dims[1] = 6;
  dims[2] = 6;
#define N_FIELDS 5
#define N_RECORDS 10
  const char *field_names[N_FIELDS] = {"a", "b", "c", "d", "e"};
  const size_t offsets[N_FIELDS] = {
      HOFFSET(blockagedata, xmin), HOFFSET(blockagedata, xmax),
      HOFFSET(blockagedata, ymin), HOFFSET(blockagedata, ymax),
      HOFFSET(blockagedata, zmin)};
  const hid_t field_types[N_FIELDS] = {
      H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT,
      H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT,
  };
  blockagedata blockages[N_RECORDS];
  for(int i = 0; i < N_RECORDS; i++) {
    blockages[i].ijk[0] = 1;
    blockages[i].ijk[1] = 1;
    blockages[i].ijk[2] = 1;
    blockages[i].ijk[3] = 1;
    blockages[i].ijk[4] = 1;
    blockages[i].ijk[5] = 1;
    blockages[i].name = "ObstName";
    blockages[i].xmin = 1.0;
    blockages[i].xmax = 1.0;
    blockages[i].ymin = 1.0;
    blockages[i].ymax = 1.0;
    blockages[i].zmin = 1.0;
    blockages[i].zmax = 1.0;
  }
  status = H5TBmake_table("Obstructions", file_id, "Obsts", N_FIELDS, N_RECORDS,
                          sizeof(blockagedata), field_names, offsets,
                          field_types, 5, 0, 0, blockages);

  // // Create the dataset.
  // hid_t dataset_id = H5Dcreate(file_id, "/slice", H5T_IEEE_F64LE,
  // dataspace_id,
  //                              H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  // double dset_data[144];
  // for(size_t i = 0; i < 4; i++) {
  //   for(size_t j = 0; j < 6; j++) {
  //     for(size_t k = 0; k < 6; k++) {
  //       size_t index = i * 6 * 6 + j * 6 + k;
  //       dset_data[index] = i * 4 + j + 23;
  //     }
  //   }
  // }
  // status = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
  //                   H5P_DEFAULT, dset_data);
  // if(status < 0) goto fail;
  // status = H5Dread(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
  // H5P_DEFAULT,
  //                  dset_data);
  // if(status < 0) goto fail;

  // status = set_quantity(dataset_id, "VELOCITY");
  // status = set_short_name(dataset_id, "vel");
  // status = set_units(dataset_id, "m/s");
  // status = add_double(dataset_id, "Time", 15.0);
  // status = add_int(dataset_id, "Mesh", 2);
  // int ijk[6] = {2, 6, 6, 12, 6, 12};
  // status = set_bounds(dataset_id, ijk);
  // if(status < 0) goto fail;

  // Close the dataset and dataspace
fail:
  // H5Dclose(dataset_id);
  // H5Sclose(dataspace_id);
  return status;
}

herr_t add_fds_version(hid_t file_id) {
  return add_string_attribute(file_id, "FDSVersion",
                              "FDS-6.8.0-0-g886e009-release");
}

herr_t add_chid(hid_t file_id) {
  return add_string_attribute(file_id, "CHID", "room_fire");
}

herr_t add_albedo(hid_t file_id) { return add_float(file_id, "Albedo", 3.0); }

int main() {
  hid_t file_id = H5Fcreate("file.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  herr_t status = 0;
  status = add_dataset(file_id);
  if(status < 0) goto fail;
  add_slice_dataset(file_id);
  if(status < 0) goto fail;
  status = add_time_bounds(file_id);
  if(status < 0) goto fail;
  status = add_fds_version(file_id);
  if(status < 0) goto fail;
  status = add_chid(file_id);
  if(status < 0) goto fail;
  status = add_albedo(file_id);
  if(status < 0) goto fail;
  add_string_attribute(file_id, "Title", "Single Couch Test Case");
  add_int(file_id, "NMeshes", 4);
  add_obsts_list(file_id);
fail:
  status = H5Fclose(file_id);
  return status;
}
