void main() {
    char *video = (char*)0xb8000;
    int counter = 0;

    video[158] = (counter % 10) + '0'; 
        video[159] = 0x0E; // YELLOW
        for(volatile int i = 0; i < 1000000; i++);
        
        counter++;
}