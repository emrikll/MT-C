#include "io.h"


int print(const char* str) {
    USART_ITConfig( USART1, USART_IT_RXNE, ENABLE);
    for (int i = 0; str[i] != '\0'; i++){
      while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      USART_SendData( USART1, str[i]);
    }
    return 0;
  }

  int printf(const char* str, ...)
  {
    // initializing list pointer 
    va_list ptr; 
    va_start(ptr, str); 
  
    // char array to store token 
    char token[1000]; 
    // index of where to store the characters of str in 
    // token 
    int k = 0; 
  
    // parsing the formatted string 
    for (int i = 0; str[i] != '\0'; i++) { 
        token[k++] = str[i]; 
  
        if (str[i + 1] == '%' || str[i + 1] == '\0') { 
            token[k] = '\0'; 
            k = 0; 
            if (token[0] != '%') { 
                char string[64];
                sprintf(
                    string, "%s", 
                    token); // printing the whole token if 
                            // it is not a format specifier
                print(string);
            } 
            else { 
                int j = 1; 
                char ch1 = 0; 
  
                // this loop is required when printing 
                // formatted value like 0.2f, when ch1='f' 
                // loop ends 
                while ((ch1 = token[j++]) < 58) { 
                } 
                // for integers 
                if (ch1 == 'i' || ch1 == 'd' || ch1 == 'u'
                    || ch1 == 'h') {
                    char string[64];
                    sprintf(string, token, 
                            va_arg(ptr, int));
                    print(string);
                } 
                // for characters 
                else if (ch1 == 'c') {
                    char string[64];
                    sprintf(string, token, 
                            va_arg(ptr, int)); 
                    print(string);
                } 
                // for float values 
                else if (ch1 == 'f') {
                    char string[64];
                    sprintf(string, token, 
                            va_arg(ptr, double));
                    print(string);
                } 
                else if (ch1 == 'l') { 
                    char ch2 = token[2]; 
  
                    // for long int 
                    if (ch2 == 'u' || ch2 == 'd'
                        || ch2 == 'i') {
                        char string[64];
                        sprintf(string, token, 
                                va_arg(ptr, long));

                        print(string);
                    } 
  
                    // for double 
                    else if (ch2 == 'f') {
                        char string[64];
                        sprintf(string, token, 
                                va_arg(ptr, double));
                        print(string);
                    } 
                } 
                else if (ch1 == 'L') { 
                    char ch2 = token[2]; 
  
                    // for long long int 
                    if (ch2 == 'u' || ch2 == 'd'
                        || ch2 == 'i') { 
                        char string[64];
                        sprintf(string, token, 
                                va_arg(ptr, long long));
                        print(string);
                    } 
  
                    // for long double 
                    else if (ch2 == 'f') {
                        char string[64];
                        sprintf(string,  token, 
                                va_arg(ptr, long double));
                        print(string);
                    } 
                } 
  
                // for strings 
                else if (ch1 == 's') {
                    char string[64];
                    sprintf(string, token, 
                            va_arg(ptr, char*)); 
                    print(string);
                } 
  
                // print the whole token 
                // if no case is matched 
                else { 
                    char string[64];
                    sprintf(string, "%s", token);
                    print(string);
                } 
            } 
        } 
    } 
  
    // ending traversal 
    va_end(ptr); 
    return 0; 
  }
