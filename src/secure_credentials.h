#ifndef SECURE_CREDENTIALS_H
#define SECURE_CREDENTIALS_H


const char CA_CERT_PROG[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
... your certificate goes here ...
-----END CERTIFICATE-----
)EOF";

const char CLIENT_CERT_PROG[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
... your certificate goes here ...
-----END CERTIFICATE-----
)EOF";

// KEEP THIS VALUE PRIVATE AND SECURE!!!
const char CLIENT_KEY_PROG[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
... your private key goes here ...
-----END RSA PRIVATE KEY-----
)KEY";


#endif //SECURE_CREDENTIALS_H
