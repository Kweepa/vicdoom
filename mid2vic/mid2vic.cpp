#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <vector>

struct SEvent
{
   int got;
   int on;
   int voice;
   int tick;
   int note;
   int realTick;
};

struct SNote
{
   int tick;
   int voice;
   int duration;
   int note;
};

SEvent try_get_event_from_file(FILE *fp, int voice, int octaveShift)
{
   SEvent e;
   e.got = false;
   char caLine[256];
   // eg "19 On ch=2 n=40 v=114"
   while (!feof(fp))
   {
      fgets(caLine, 255, fp);
      if (sscanf(caLine, "%d On ch=%*d n=%d v=%*d", &e.tick, &e.note) == 2)
      {
         e.on = true;
         e.got = true;
         break;
      }
      else if (sscanf(caLine, "%d Off ch=%*d n=%d v=%*d", &e.tick, &e.note) == 2)
      {
         e.on = false;
         e.got = true;
         break;
      }
   }

   if (e.got)
   {
      // scale it to jiffy units (original bpm was 141)
      e.tick = (180*e.tick)/(141*3);
      // adjust the octave
      e.note = e.note - 12*(voice+1-octaveShift);
      if (e.note < 0)
      {
         e.note = 0;
         printf("Try shifting up an octave!\n");
      }
      if (e.note > 35)
      {
         e.note = 35;
         printf("Try shifting down an octave!\n");
      }
      // set the voice
      e.voice = voice;
      e.realTick = e.tick;
   }

   return e;
}

int main(int argc, char *argv[])
{
   std::vector<SEvent> events;

   char caFileName[256];

   if (argc < 2)
   {
      printf("mid2vic <mapname> [<octave shift>]\n");
      return 0;
   }

   sprintf(caFileName, "%ssim_v1.txt", argv[1]);
   FILE *fp1 = fopen(caFileName, "rt");
   sprintf(caFileName, "%ssim_v2.txt", argv[1]);
   FILE *fp2 = fopen(caFileName, "rt");
   sprintf(caFileName, "%ssim_v3.txt", argv[1]);
   FILE *fp3 = fopen(caFileName, "rt");

   int octaveShift = 0;
   if (argc > 2)
   {
      octaveShift = atoi(argv[2]);
   }

   if (fp1 != NULL && fp2 != NULL && fp3 != NULL)
   {
      int lastTick = 0;
      while (!feof(fp1) || !feof(fp2) || !feof(fp3))
      {
         int pos1 = ftell(fp1), pos2 = ftell(fp2), pos3 = ftell(fp3);

         SEvent e;
         e.got = false;
         SEvent e1 = try_get_event_from_file(fp1, 1, octaveShift);
         SEvent e2 = try_get_event_from_file(fp2, 2, octaveShift);
         SEvent e3 = try_get_event_from_file(fp3, 3, octaveShift);
//          e2.got = false; // remove this track
//          e3.got = false; // remove this track

         if (e1.got && (!e2.got || e1.tick <= e2.tick) && (!e3.got || e1.tick <= e3.tick))
         {
            e = e1;
            // next note is on voice 1
            fseek(fp2, pos2, SEEK_SET);
            fseek(fp3, pos3, SEEK_SET);
         }
         else if (e2.got && (!e3.got || e2.tick <= e3.tick))
         {
            e = e2;
            // next note is on voice 2
            fseek(fp1, pos1, SEEK_SET);
            fseek(fp3, pos3, SEEK_SET);
         }
         else if (e3.got)
         {
            e = e3;
            // next note is on voice 3
            fseek(fp1, pos1, SEEK_SET);
            fseek(fp2, pos2, SEEK_SET);
         }
         if (e.got)
         {
            lastTick = e.tick;
            events.push_back(e);
         }
         else
         {
            break;
         }
      }
      if (!events.empty())
      {
         int i;
         for (i = 1; i < events.size(); ++i)
         {
            if (events[i].tick <= events[i-1].tick)
            {
               ++events[i].tick;
               // restart
               i = 1;
            }
         }
         for (i = 0; i < events.size() - 1; ++i)
         {
            events[i].tick = events[i+1].tick - events[i].tick;
         }
         events[i].tick = 1;

         // insert extra (effectively NOP) events if any intervals are > 32
         std::vector<SEvent>::iterator it = events.begin();
         while (it != events.end())
         {
            if (it->tick > 32)
            {
               SEvent ei = *it;
               ei.tick = 32;
               it->tick -= 32;
               if (it->tick == 0)
               {
                  // make sure there are no zeros
                  ei.tick = 31;
                  it->tick = 1;
               }
               events.insert(it, ei);
               // restart (ew)
               it = events.begin();
            }
            else
            {
               ++it;
            }
         }

         printf("Events = %d\n", events.size());
      }
      fclose(fp1);
      fclose(fp2);
      fclose(fp3);
   }

   // convert to a list of notes with durations
   std::vector<SNote> notes;
   std::vector<SEvent>::iterator it = events.begin();
   int tick = 0;
   while (it != events.end())
   {
      SEvent e = *it;
      if (e.on)
      {
         // look forward to the corresponding off event
         int duration = e.tick;
         std::vector<SEvent>::iterator it2 = it + 1;
         while (it2 != events.end())
         {
            SEvent e2 = *it2;
            if (!e2.on && e.voice == e2.voice)
            {
               SNote note;
               note.tick = e.realTick;
               note.voice = e.voice;
               note.note = e.note;
               note.duration = duration;
               notes.push_back(note);
               break;
            }
            duration += e2.tick;
            ++it2;
         }
      }
      tick += e.tick;
      ++it;
   }

   int songSize = 0;
   if (!events.empty())
   {
      // now write out the data
      sprintf(caFileName, "%smus.s", argv[1]);
      FILE *fp = fopen(caFileName, "wt");
      sprintf(caFileName, "%smus", argv[1]);
      FILE *fpb = fopen(caFileName, "wb");
      if (fp != NULL && fpb != NULL)
      {
         fprintf(fp, ".export startOfSong\n");
         fprintf(fp, "startOfSong:\n");
         std::vector<SEvent>::iterator it = events.begin();
         while (it != events.end())
         {
            fprintf(fp, ".byte ");
            for (int x = 0; x < 20 && it != events.end(); ++x)
            {
               if (x > 0)
               {
                  fprintf(fp, ", ");
               }
               int statusByte = (it->on ? 0x00 : 0x80);
               statusByte |= (it->voice - 1)<<5;
               statusByte |= (it->tick - 1);
               fprintf(fp, "$%02x", statusByte);
               fputc(statusByte, fpb);
               ++songSize;
               if (it->on)
               {
                  fprintf(fp, ", $%02x", it->note);
                  fputc(it->note, fpb);
                  ++songSize;
               }
               ++it;
            }
            fprintf(fp, "\n");
         }
         fprintf(fp, ".byte $ff\n");
         fputc(0xff, fpb);
         ++songSize;
         fprintf(fp, "; song size %d\n", songSize);
         fclose(fp);
         fclose(fpb);
      }
   }
}