#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 6; j++) {
      dset_data[i * 6 + j] = i * 4 + j + 23;
    }
  }
  status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    dset_data);
  if (status < 0) goto fail;
  status = H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                   dset_data);
  if (status < 0) goto fail;

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
  if (status < 0) goto fail_set_size;
  // Create a dataset attribute.
  hid_t attribute_id = H5Acreate2(file_id, attr_name, attr_type, attr_space,
                                  H5P_DEFAULT, H5P_DEFAULT);

  // Write the attribute data.
  status = H5Awrite(attribute_id, attr_type, attr_data);
  if (status < 0) goto fail;
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
  if (status < 0) goto fail_set_size;
  // Create a dataset attribute.
  hid_t attribute_id = H5Acreate2(file_id, attr_name, H5T_IEEE_F32LE,
                                  attr_space, H5P_DEFAULT, H5P_DEFAULT);
  // Write the attribute data.
  status = H5Awrite(attribute_id, H5T_IEEE_F32LE, &attr_value);
  if (status < 0) goto fail;
fail:
  // Close the attribute.
  H5Aclose(attribute_id);
fail_set_size:
  // Close the dataspace.
  H5Sclose(attr_space);
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
  if (status < 0) goto fail;
  status = add_time_bounds(file_id);
  if (status < 0) goto fail;
  status = add_fds_version(file_id);
  if (status < 0) goto fail;
  status = add_chid(file_id);
  if (status < 0) goto fail;
  status = add_albedo(file_id);
  if (status < 0) goto fail;
fail:
  status = H5Fclose(file_id);
  return status;
}
