         Bitmap b = new Bitmap("c:\\dev\vicdoom\\doomfont.png");
         if (b != null)
         {
            TextWriter t = new StreamWriter("c:\\dev\\vicdoom\\doomfont.s");
            if (t != null)
            {
               t.WriteLine(".segment \"UDG\"");
               // loop over and write out for assembly include
               for (int y = 0; y < 8; ++y)
               {
                  t.WriteLine();
                  for (int x = 0; x < 8; ++x)
                  {
                     t.Write(".byte ");
                     for (int z = 0; z < 8; ++z)
                     {
                        int f = 0;
                        for (int a = 0; a < 8; ++a)
                        {
                           f <<= 1;
                           Color c = b.GetPixel(8*x + a, + 8*y + z);
                           if (c.R > 100 | c.G > 100 | c.B > 100)
                           {
                              f += 1;
                           }
                        }
                        if (z < 7)
                        {
                           t.Write("${0:x2}, ", f);
                        }
                        else
                        {
                           t.WriteLine("${0:x2}", f);
                        }
                     }
                  }
               }
               t.Close();
            }
         }

