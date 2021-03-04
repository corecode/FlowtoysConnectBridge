// template <typename Iterator>
// std::string join(Iterator begin, Iterator end, char separator = '.')
// {
//     std::ostringstream o;
//     if(begin != end)
//     {
//         o << *begin++;
//         for(;begin != end; ++begin)
//             o  << separator << *begin;
//     }
//     return o.str();
// }
//  
// template <typename Container>
// std::string join(Container const& c, char separator = '.') // can pass array as reference, too
// {
//     using std::begin;
//     using std::end;
//     return join(begin(c), end(c), separator);
//     // not using std::... directly:
//     // there might be a non-std overload that wouldn't be found if we did
// }

int to_int(char const *s) {
  if ( s == NULL || *s == '\0' )
   throw std::invalid_argument("null or empty string argument");
 
  bool negate = (s[0] == '-');
  if ( *s == '+' || *s == '-' ) 
  ++s;
 
  if ( *s == '\0')
   throw std::invalid_argument("sign character only.");
 
  int result = 0;
  while(*s) {
   if ( *s < '0' || *s > '9' )
     throw std::invalid_argument("invalid input string");
   result = result * 10  - (*s - '0');  //assume negative number
   ++s;
  }
  return negate ? result : -result; //-result is positive!
} 

void print_bytes( void *ptr, size_t size )
{
    //char *buf = (char*) ptr;
    unsigned char *p = (unsigned char*)ptr ;

    for( size_t i = 0; i < size; i++ )
    {
        printf( "%02hhX ", p[i] ) ;
    }
    printf( "\n" ) ;

    for( size_t i = 0; i < size; i++ )
    {
        for( short j = 7; j >= 0; j-- )
        {
            printf( "%d", ( p[i] >> j ) & 1 ) ;
        }
        printf(" ");
    }
    printf("\n");
}

char* toHex(void *ptr, size_t size) {
    // //char *buf = (char*) ptr;
    // unsigned char *p = (unsigned char*)ptr ;
    //
    // for( size_t i = 0; i < size; i++ ) {
    //     printf( "%02hhX ", p[i] ) ;
    // }
  return "hex"; 
}
