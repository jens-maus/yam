int stch_i(const char *s,int *res)
{
   int i=0;

   *res=0;
   while(s[i])
   {
      if(s[i]>='0' && s[i]<='9')
      {
         *res<<=4;
         *res+=s[i++]-'0';
         continue;
      }
      if(s[i]>='a' && s[i]<='f')
      {
         *res<<=4;
         *res+=s[i++]-'a'+10;
         continue;
      }
      if(s[i]>='A' && s[i]<='F')
      {
         *res<<=4;
         *res+=s[i++]-'A'+10;
         continue;
      }
      break;
   }

   return i;
}
