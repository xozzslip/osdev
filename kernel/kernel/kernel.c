void dummy() {
    dummy();  // do not enter there accidentelly
    // purpose of this function is to be forced to
    // call main instead of jumping into this file
}

int main() {
    char *video_memory = (char *) (0xb8000 + 2 * (14 * 80 + 2));
    *video_memory = 'Z';
    return 0;
}