#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>



typedef struct
{
    double      hue;
    double      sat;
    double      brt;
    uint8_t  run;
    uint32_t cnt;
} ATOM;

class Canvas
{
  public:
    Canvas(int w, int h)
      : dsWidth(w)
      , dsHeight(h)
    {
        ds.resize(width() * height());
        clear();
    }

    ~Canvas()
    {
    }

    void setValues(int x, int y, const ATOM &atom)
    {
        if ((y < 0) || (x < 0) || (x >= width()) || (y >= height()))
            return;
        auto p = x + y * width();
        ds[p]  = atom;
    }

    void getValues(int x, int y, ATOM &atom) const
    {
        int32_t p;
        if ((y < 0) || (x < 0) || (x >= width()) || (y >= height()))
        {
            atom.hue = 1.0;
            atom.sat = 1.0;
            atom.brt = 1.0;
            atom.run = 0xFF;
            atom.cnt = 0;
            return;
        }
        p    = x + y * width();
        atom = ds[p];
    }

    void clear(void)
    {
        int32_t p = 0;
        for (int32_t y = 0; y < height(); y++)
        {
            for (int32_t x = 0; x < width(); x++)
            {
                ds[p].hue = NAN;
                ds[p].sat = NAN;
                ds[p].brt = NAN;
                ds[p].run = 0xff;
                ds[p].cnt = 0;
                p++;
            }
        }
    }
    int32_t width() const
    {
        return dsWidth;
    }

    int32_t height() const
    {
        return dsHeight;
    }

    static void HSVtoRGB(double &r, double &g, double &b, double h, double s, double v)
    {
        int    i;
        double f, p, q, t;

        while (h >= 1.0)
        {
            h -= 1.0;
        }
        while (h < .0)
        {
            h += 1.0;
        }

        if (s == 0.0)
        {
            r = g = b = v;
            return;
        }
        h = h * 6.0; // sector 0 to 5
        i = ((int) floor(h)) % 6;
        f = h - i; // factorial part of h

        p = v * (1.0 - s);
        q = v * (1.0 - s * f);
        t = v * (1.0 - s * (1.0 - f));
        switch (i)
        {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            case 5:
            default:
                r = v;
                g = p;
                b = q;
                break;
        }
    }

    void saveAsRaw(const std::string &filename) const
    {
        FILE *fp;
        fp = fopen(filename.c_str(), "wb");
        uint32_t maxCnt[256];
        memset(maxCnt, 0, sizeof(maxCnt));
        printf("saving %d x %d\n", width(), height());
        for (int y = 0; y < height(); y++)
        {
            int32_t  p=y*width();
            for (int x = 0; x < width(); ++x, ++p)
            {
                if (ds[p].cnt > maxCnt[ds[p].run])
                {
                    maxCnt[ds[p].run] = ds[p].cnt;
                }
                double r, g, b;
                HSVtoRGB(r, g, b, ds[p].hue, ds[p].sat, ds[p].brt);
                uint16_t vals[3] = {static_cast<uint16_t>(r * 65535.0), static_cast<uint16_t>(g * 65535.0),
                                    static_cast<uint16_t>(b * 65535.0)};
                fwrite(vals, 1, sizeof(vals), fp);
            }
        }
        fclose(fp);
        for (int i = 0; i < 10; ++i)
        {
            printf("MaxCnt %d = %u\n", i, maxCnt[i]);
        }
    }

  private:
    std::vector<ATOM> ds;
    int32_t dsWidth;
    int32_t dsHeight;
};
