#include <cinttypes>

/// Bundle of root certificate
// https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/README.md
extern const uint8_t rootca_crt_bundle[] asm("_binary_data_cert_x509_crt_bundle_bin_start");

// Failed to verify Letsencrypt cert with ISRG Root X1 CA (IDFGH-11039)
// https://github.com/espressif/arduino-esp32/issues/8626

// Google Trust Services https://pki.goog/repository/
/// GTS Root R1, valid until 2036-06-22
extern const char gts_root_r1_crt[] asm("_binary_data_cert_gts_root_r1_pem_start");
/// GTS Root R4, Valid Until 2036-06-22
extern const char gts_root_r4_crt[] asm("_binary_data_cert_gts_root_r4_pem_start");

// https://support.globalsign.com/ca-certificates/intermediate-certificates/organizationssl-intermediate-certificates
/// GlobalSign RSA Organization Validation CA - 2018, Valid until: 21 November 2028
extern const char gsrsaovsslca2018_crt[] asm("_binary_data_cert_gsrsaovsslca2018_pem_start");
