void _start() {
    char* video = (char*)0xB8000;
    video[0] = 'A'; 
    while(1);
}