#include "Fingerprint.h"
#include <unistd.h>
#include <uuid/uuid.h>
#include <array>

FingerprintData collect_fingerprint() {
    FingerprintData data;
    uuid_t id;
    uuid_generate(id);
    char uuid_str[37];
    uuid_unparse(id, uuid_str);
    data.uuid = uuid_str;
    char host[256];
    gethostname(host, sizeof(host));
    data.hostname = host;
    return data;
}
