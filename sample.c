int netascii_decode(const char *in, int in_len, char *out) {
    int j = 0;
    for (int i = 0; i < in_len; i++) {
        if (in[i] == '\r') {
            if (i+1 < in_len && in[i+1] == '\n') {
                #ifdef TARGET_OS_LINUX
                    out[j++] = '\n';   // collapse CRLF → LF
                #elif defined(TARGET_OS_WINDOWS)
                    out[j++] = '\r';   // keep CRLF
                    out[j++] = '\n';
                #elif defined(TARGET_OS_MAC)
                    out[j++] = '\r';   // collapse CRLF → CR
                #endif
                i++; // skip next char
            } else if (i+1 < in_len && in[i+1] == '\0') {
                out[j++] = '\r';       // CRNUL → CR
                i++;
            } else {
                out[j++] = '\r';       // lone CR
            }
        } else {
            out[j++] = in[i];
        }
    }
    return j;
}
