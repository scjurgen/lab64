
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <json.hpp>

#include "Canvas.h"
#include "RandomGenerator.h"

RandomGenerator rndg;

static struct
{
    int8_t x;
    int8_t y;
} dirPlus[8] = {{-1, 1}, {1, -1}, {1, 1}, {-1, -1}, {0, 1}, {0, -1}, {-1, 0}, {1, 0}};

class Lab
{
    int  crystalUndisturbed = 1;
    int  minLength          = 400;
    int  maxLength          = 1200;
    bool addOnStraight      = true;
    bool shuffle            = false;
    int  garbageStart       = 10;

    double maxHueAdd = 0;
    double maxSatAdd = 0;
    double maxBrtAdd = 0;
    double minHueAdd = 2;
    double minSatAdd = 2;
    double minBrtAdd = 3;

    uint32_t activePoints = 1;

    uint32_t pos = 0;
    int32_t  xS[100], yS[100];
    double   maxHue[100], minHue[100];
    double   maxBrt[100], minBrt[100];
    double   maxSat[100], minSat[100];

    ATOM     atom;
    uint32_t lastrun     = 0;
    uint32_t itemsInList = 0;

    typedef struct runlist
    {
        int16_t x, y;
        int16_t dir; // direction to crystalize (referenced by dirPlus)
        int16_t len; // length to crystalize this run
        int16_t set; // from which initial nucleus
    } RUNLIST;

    std::vector<RUNLIST> rl;

  public:
    Canvas canvas;

    uint32_t newRuns         = 0;
    uint32_t maximumListUsed = 0;

    Lab(int32_t w, int32_t h)
      : canvas(w, h)
    {
        rl.resize(1000000);
    }

    size_t listSize() const
    {
        return rl.size();
    }

    size_t maxDirections()
    {
        return sizeof(dirPlus) / sizeof(dirPlus[0]);
    }

    void resetRuns(void)
    {
        itemsInList = 0;
        lastrun     = 0;

        for (size_t n = 0; n < listSize(); n++)
        {
            rl[n].x = -1;
        }
    }

    void clampValue(double &target, double min, double max)
    {
        if (target < min)
            target = min;
        if (target > max)
            target = max;
    }

    void torusValue(double &target, double min, double max)
    {
        if (target < min)
            target += (max - min);
        if (target > max)
            target -= (max - min);
    }

    void addColor(double &r, double &g, double &b, double addr, double addg, double addb, int set)
    {
        double nH, nS, nB;
        nH = r + addr;
        nS = g + addg;
        nB = b + addb;
        if ((maxHue[set] == 1.0) && (minHue[set] == 0.0))
            torusValue(nH, minHue[set], maxHue[set]);
        else
            clampValue(nH, minHue[set], maxHue[set]);
        clampValue(nS, minSat[set], maxSat[set]);
        clampValue(nB, minBrt[set], maxBrt[set]);
        r = nH;
        g = nS;
        b = nB;
    }

    void garbageCollect(void)
    {
        uint32_t i = 0, j = 0;
        while (i < lastrun)
        {
            if (rl[i].x != -1) // not used entries gets skipped, only copy good items
            {
                rl[j++] = rl[i];
            }
            i++;
        }
        lastrun     = j;
        itemsInList = j;
    }

    void addRunner(int x, int y, int32_t dir, int32_t len, int32_t set)
    {
        int32_t n = 0;
        atom.run  = set;
        canvas.setValues(x, y, atom);
        if (lastrun >= listSize() - 1)
        {
            garbageCollect();
        }

        if (itemsInList >= listSize() - 1)
        {
            printf("Too many items in list\n");
            itemsInList = 0;
            return;
        }
        if (pos < lastrun)
        {
            n = pos;
            if (rl[n].x == -1)
            {
                rl[n].x   = x;
                rl[n].y   = y;
                rl[n].dir = dir;
                rl[n].len = len;
                ++itemsInList;
                return;
            }
        }
        n = lastrun;
        lastrun++;
        rl[n].x   = x;
        rl[n].y   = y;
        rl[n].dir = dir;
        rl[n].len = len;
        ++itemsInList;
        if (itemsInList > maximumListUsed)
        {
            maximumListUsed = itemsInList;
        }
    }

    void GetNewRuns(int32_t x, int32_t y, int32_t set)
    {
        uint32_t nR[maxDirections()];
        ATOM     oAtom = atom;
        newRuns++;
        for (uint32_t dir = 0; dir < maxDirections(); dir++)
        {
            nR[dir] = dir;
        }

        for (uint32_t dir = 0; dir < maxDirections(); dir++)
        { // shuffle directions index
            int i   = rndg.GetNormalizedUniformRange() * maxDirections();
            int a   = nR[dir];
            nR[dir] = nR[i];
            nR[i]   = a;
        }
        for (uint32_t d = 0; d < maxDirections(); d++)
        {
            uint32_t dir = nR[d];
            ATOM     nAtom;
            canvas.getValues(x + dirPlus[dir].x, y + dirPlus[dir].y, nAtom);
            if (std::isnan(nAtom.hue))
            {
                atom = oAtom;
                atom.cnt++;
                if (!addOnStraight)
                {
                    addColor(atom.hue, atom.sat, atom.brt, rndg.GetUniformRange(minHueAdd, maxHueAdd),
                             rndg.GetUniformRange(minSatAdd, maxSatAdd), rndg.GetUniformRange(minBrtAdd, maxBrtAdd),
                             set);
                }
                auto nx = x + dirPlus[dir].x;
                auto ny = y + dirPlus[dir].y;
                auto r1 = pow(rndg.GetNormalizedUniformRange(), 2);
                auto r2 = r1 * static_cast<double>(maxLength - minLength);
                r2 += minLength;
                addRunner(nx, ny, dir, (int32_t) r2, set);
            }
        }
    }

    uint32_t crystallizeBatch(uint32_t idx)
    {
        while (idx < lastrun)
        {
            if (rl[idx].x != -1)
            {
                int32_t x, y, dir, len, set;
                bool    needsNewRun;
                int32_t c = 0;
                do
                {
                    x   = rl[idx].x;
                    y   = rl[idx].y;
                    dir = rl[idx].dir;
                    len = rl[idx].len;
                    set = rl[idx].set;
                    canvas.getValues(x, y, atom);
                    atom.cnt++;
                    if (addOnStraight) // change color only for every active point
                        addColor(atom.hue, atom.sat, atom.brt, rndg.GetUniformRange(minHueAdd, maxHueAdd),
                                 rndg.GetUniformRange(minSatAdd, maxSatAdd), rndg.GetUniformRange(minBrtAdd, maxBrtAdd),
                                 set);
                    // some slight color changes on pixel run
                    x += dirPlus[dir].x;
                    y += dirPlus[dir].y;
                    needsNewRun = false;
                    ATOM nAtom;
                    canvas.getValues(x, y, nAtom);
                    if (!std::isnan(nAtom.hue))
                    {
                        needsNewRun = true;
                    } // can't continue: split pathes
                    len--;
                    if (len < 0)
                    {
                        needsNewRun = true;
                    } // end of voyage: split pathes
                    if (!needsNewRun)
                    {
                        atom.run = set;
                        canvas.setValues(x, y, atom);
                        rl[idx].x   = x;
                        rl[idx].y   = y;
                        rl[idx].len = len;
                    }
                    c++;
                    if (c >= crystalUndisturbed)
                    { // break after some steps
                        break;
                    }
                    if (len < 1)
                    {
                        break;
                    }
                } while (!needsNewRun);
                if (needsNewRun)
                {
                    return getNewRuns(idx, x, y, dir, set);
                }
                if (shuffle)
                {
                    return (int32_t)(rndg.GetNormalizedUniformRange() * lastrun);
                }
                return idx;
            }
            idx++;
        }
        return 0;
    }

    uint32_t getNewRuns(uint32_t idx, int32_t x, int32_t y, int32_t dir, int32_t set)
    {
        x -= dirPlus[dir].x; // back one pixel
        y -= dirPlus[dir].y;
        rl[idx].x = -1;
        --itemsInList;
        if (itemsInList < lastrun - garbageStart)
        {
            garbageCollect();
        }
        if (idx >= lastrun)
        {
            idx = lastrun - 1;
        }
        GetNewRuns(x, y, set);
        if (shuffle)
        {
            if (lastrun)
            {
                return (int32_t)(rndg.GetNormalizedUniformRange() * lastrun);
            }
            else
            {
                return idx + 1;
            }
        }
        return idx;
    }

    int32_t crystallize()
    {
        int32_t i = 0;

        if (!itemsInList)
        {
            return 0;
        }
        do
        {
            pos = crystallizeBatch(pos);
            if (pos >= lastrun)
            {
                pos = 0;
            }
            i++;
            if (!itemsInList)
            {
                return 0;
            }
        } while (pos && (i < 100));
        if (!itemsInList)
        {
            return 0;
        }
        return 1;
    }

    void ReadParams(nlohmann::json &json)
    {
        activePoints       = json["activePoints"].get<int>();
        minHueAdd          = -1 * json["rangeHue"].get<double>();
        maxHueAdd          = json["rangeHue"].get<double>() + json["biasHue"].get<double>();
        minSatAdd          = -1 * json["rangeSat"].get<double>();
        maxSatAdd          = json["rangeSat"].get<double>() + json["biasSat"].get<double>();
        minBrtAdd          = -1 * json["rangeBrt"].get<double>();
        maxBrtAdd          = json["rangeBrt"].get<double>() + json["biasBrt"].get<double>();
        addOnStraight      = json["addOnStraight"].get<bool>();
        minLength          = json["minLength"].get<int>();
        maxLength          = json["maxLength"].get<int>();
        shuffle            = json["shuffle"].get<int>();
        garbageStart       = json["garbageStart"].get<int>();
        crystalUndisturbed = json["crystalUndisturbed"].get<int>();

        resetRuns();
        canvas.clear();
        for (uint32_t i = 0, t = 0; (i < activePoints) && (t < 1000); t++)
        {
            ATOM nAtom;
            try
            {
                auto valx = json["points"][i]["x"].get<double>();
                xS[i]     = valx * canvas.width();
            }
            catch (nlohmann::json::exception &e)
            {
                xS[i] = rndg.GetNormalizedUniformRange() * canvas.width();
                xS[i] = xS[i] / 2 + canvas.width() / 4;
            }
            try
            {
                auto valy = json["points"][i]["y"].get<double>();
                yS[i]     = valy * canvas.height();
            }
            catch (std::exception &e)
            {
                yS[i] = rndg.GetNormalizedUniformRange() * canvas.height();
                yS[i] = yS[i] / 2 + canvas.height() / 4;
            }

            minHue[i] = json["points"][i]["hue"]["min"].get<double>();
            maxHue[i] = json["points"][i]["hue"]["max"].get<double>();

            minSat[i] = json["points"][i]["sat"]["min"].get<double>();
            maxSat[i] = json["points"][i]["sat"]["max"].get<double>();

            minBrt[i] = json["points"][i]["brt"]["min"].get<double>();
            maxBrt[i] = json["points"][i]["brt"]["max"].get<double>();
            printf("xy(%d)=[%d %d]\n", i, xS[i], yS[i]);

            canvas.getValues(xS[i], yS[i], nAtom);
            if (std::isnan(nAtom.hue))
            {
                atom.hue = json["points"][i]["hue"]["init"].get<double>();
                atom.sat = json["points"][i]["sat"]["init"].get<double>();
                atom.brt = json["points"][i]["brt"]["init"].get<double>();
                atom.run = i;
                atom.cnt = 1;
                canvas.setValues(xS[i], yS[i], atom);
                GetNewRuns(xS[i], yS[i], i);
                i++;
            }
        }
    }
};

int main(int ac, char *av[])
{
    if (ac == 1)
    {
        printf("Usage: %s <parameters>\n", av[0]);
        printf("-j,--json          parameter file json\n");
        printf("-o,--output        image output file (raw unsigned 16bit RGB)\n");
        printf("-w,--width         width\n");
        printf("-h,--height        height\n");
        printf("-s,--oversample    oversample factor (2,3,4)\n");
        printf("-r,--randseed      random seed value\n");
    }
    else
    {
        int         oversample = 1;
        std::string jsonFilename;
        std::string outputFilename("test.raw");
        int         w = 3840, h = 2400;
        int         idxAc = 1;
        while (idxAc < ac)
        {
            std::string item(av[idxAc++]);
            if (item == "-j" || item == "--json")
            {
                jsonFilename = av[idxAc++];
            }
            else if (item == "-o" || item == "--output")
            {
                outputFilename = av[idxAc++];
            }
            else if (item == "-w" || item == "--width")
            {
                w = ::atoi(av[idxAc++]);
            }
            else if (item == "-h" || item == "--height")
            {
                h = ::atoi(av[idxAc++]);
            }
            else if (item == "-s" || item == "--oversample")
            {
                oversample = ::atoi(av[idxAc++]);
            }
            else if (item == "-r" || item == "--randseed")
            {
                rndg.seed(::atoi(av[idxAc++]));
            }
            else
            {
                std::cout << "unknown parameter: " << item << std::endl;
                exit(-2);
            }
        }

        std::ifstream ifs(jsonFilename);
        if (!ifs.is_open())
        {
            std::cout << "couldn't open json file: " << jsonFilename << std::endl;
            return -1;
        }
        nlohmann::json j;
        ifs >> j;
        int ret = 0;

        Lab lab(w * oversample, h * oversample);
        lab.ReadParams(j);
        do
        {
            ret = lab.crystallize();
        } while (ret);
        printf("Maximum list depth: %d\n", lab.maximumListUsed);
        std::stringstream ssRaw;
        ssRaw << outputFilename << ".raw";
        lab.canvas.saveAsRaw(ssRaw.str().c_str());
        FILE *fp = fopen("to-jpeg.sh", "w");
        fprintf(fp, "#!/bin/bash\n");
        std::stringstream ss;
        ss << outputFilename << ".jpg";
        if (oversample == 1)
            fprintf(fp, "convert -size %dX%d -depth 16 rgb:%s %s\n", w, h, ssRaw.str().c_str(), ss.str().c_str());
        else
            fprintf(fp, "convert -size %dX%d -depth 16 -resize %dx%d rgb:%s %s\n", w * oversample, h * oversample, w, h,
                    ssRaw.str().c_str(), ss.str().c_str());
        fprintf(fp, "open -a Preview %s\n\n", ss.str().c_str());
        fprintf(fp, "xdg-open %s\n\n", ss.str().c_str());
        fclose(fp);

        printf("New runs: %d\n", lab.newRuns);
        system("chmod +x ./to-jpeg.sh");
        system("./to-jpeg.sh");
    }
    return 0;
}
