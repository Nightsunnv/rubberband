/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Rubber Band Library
    An audio time-stretching and pitch-shifting library.
    Copyright 2007-2024 Particular Programs Ltd.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.

    Alternatively, if you have a valid commercial licence for the
    Rubber Band Library obtained by agreement with the copyright
    holders, you may redistribute and/or modify it under the terms
    described in that licence.

    If you wish to distribute code using the Rubber Band Library
    under terms other than those of the GNU General Public License,
    you must obtain a valid commercial licence before doing so.
*/

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/unit_test.hpp>

#include "../../rubberband/RubberBandLiveShifter.h"

#include <iostream>

#include <cmath>

using namespace RubberBand;

using std::vector;
using std::cerr;
using std::endl;

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_SUITE(TestLiveShifter)

BOOST_AUTO_TEST_CASE(sinusoid_unchanged)
{
#ifdef TO_BE_CONTINUED
    
    bool printDebug = true;
//    bool printDebug = false;
    
    int n = 100000;
    float freq = 440.f;
    int rate = 44100;

    if (printDebug) {
        RubberBandLiveShifter::setDefaultDebugLevel(2);
    }
    
    RubberBandLiveShifter shifter
        (rate, 1, RubberBandLiveShifter::OptionPitchModeB);

    //!!!
//    shifter.setPitchScale(2.0);
    
    int blocksize = shifter.getBlockSize();
    BOOST_TEST(blocksize == 512);

    n = (n / blocksize + 1) * blocksize;
    
    vector<float> in(n), out(n);
    for (int i = 0; i < n; ++i) {
        in[i] = sinf(float(i) * freq * M_PI * 2.f / float(rate));
    }

    for (int i = 0; i < n; i += blocksize) {
        float *inp = in.data() + i;
        float *outp = out.data() + i;
        shifter.shift(&inp, &outp);
    }

    int delay = shifter.getStartDelay();

    std::cerr << "delay reported as " << delay << std::endl;    
    
    // We now have n samples of a simple sinusoid with stretch factor
    // 1.0; obviously we expect the output to be essentially the same
    // thing. It will have lower precision for a while at the start
    // and end because of windowing factors, so we check those with a
    // threshold of 0.1; in the middle we expect better
    // precision. Note that these are relative tolerances, not
    // absolute, i.e. 0.001 means 0.001x the smaller value - so they
    // are tighter than they appear.

    // This syntax for comparing containers with a certain tolerance
    // using BOOST_TEST is just bonkers. I can't find the << syntax to
    // combine manipulators documented anywhere other than in a
    // release note, but it does work. Well, sort of - it works this
    // way around but not as per_element << tolerance. And
    // tolerance(0.1) doesn't do what you'd expect if the things
    // you're comparing are floats (it sets the tolerance for doubles,
    // leaving float comparison unchanged). Clever... too clever.

//    BOOST_TEST(out == in,
//               tt::tolerance(0.1f) << tt::per_element());

    BOOST_TEST(vector<float>(out.begin() + delay, out.begin() + n) ==
               vector<float>(in.begin(), in.begin() + n - delay),
               tt::tolerance(0.001f) << tt::per_element());

    if (printDebug) {
        // The prefix is to allow grep on the test output
        
        std::cout << "IN,sample,V" << std::endl;
        for (int i = 0; i < int(in.size()); ++i) {
            std::cout << "IN," << i << "," << in[i] << std::endl;
        }
        
        std::cout << "OUT,sample,V" << std::endl;
        for (int i = 0; i < int(out.size()); ++i) {
            std::cout << "OUT," << i << "," << out[i] << std::endl;
        }

        std::cout << "DIFF,V" << std::endl;
        for (int i = 0; i + delay < int(in.size()); ++i) {
            std::cout << "DIFF," << i << "," << out[i + delay] - in[i] << std::endl;
        }
    }
#endif
}

//!!!
#ifdef NOT_DEFINED

BOOST_AUTO_TEST_CASE(sinusoid_unchanged_offline_finer)
{
    int n = 10000;
    float freq = 440.f;
    int rate = 44100;

    RubberBandLiveShifter shifter
        (rate, 1, RubberBandLiveShifter::OptionEngineFiner);
    
    vector<float> in(n), out(n);
    for (int i = 0; i < n; ++i) {
        in[i] = sinf(float(i) * freq * M_PI * 2.f / float(rate));
    }
    float *inp = in.data(), *outp = out.data();

    shifter.setMaxProcessSize(n);
    shifter.setExpectedInputDuration(n);
    BOOST_TEST(shifter.available() == 0);

    shifter.study(&inp, n, true);
    BOOST_TEST(shifter.available() == 0);

    shifter.process(&inp, n, true);
    BOOST_TEST(shifter.available() == n);

    BOOST_TEST(shifter.getStartDelay() == 0); // offline mode
    
    size_t got = shifter.retrieve(&outp, n);
    BOOST_TEST(got == n);
    BOOST_TEST(shifter.available() == -1);

    // The R3 engine is actually less precise than R2 here because of
    // its different windowing design, though see the note above about
    // what these tolerances mean
    
    BOOST_TEST(out == in,
               tt::tolerance(0.35f) << tt::per_element());
    
    BOOST_TEST(vector<float>(out.begin() + 1024, out.begin() + n - 1024) ==
               vector<float>(in.begin() + 1024, in.begin() + n - 1024),
               tt::tolerance(0.01f) << tt::per_element());

//    std::cout << "ms\tV" << std::endl;
//    for (int i = 0; i < n; ++i) {
//        std::cout << i << "\t" << out[i] - in[i] << std::endl;
//    }
}

BOOST_AUTO_TEST_CASE(sinusoid_2x_offline_finer)
{
    int n = 10000;
    float freq = 441.f; // so a cycle is an exact number of samples
    int rate = 44100;

    RubberBandLiveShifter shifter
        (rate, 1, RubberBandLiveShifter::OptionEngineFiner);

    shifter.setTimeRatio(2.0);
    
    vector<float> in(n), out(n*2);
    for (int i = 0; i < n*2; ++i) {
        out[i] = sinf(float(i) * freq * M_PI * 2.f / float(rate));
        if (i < n) {
            in[i] = out[i];
        }
    }
    float *inp = in.data(), *outp = out.data();

    shifter.setMaxProcessSize(n);
    shifter.setExpectedInputDuration(n);
    BOOST_TEST(shifter.available() == 0);

    shifter.study(&inp, n, true);
    BOOST_TEST(shifter.available() == 0);

    shifter.process(&inp, n, true);
    BOOST_TEST(shifter.available() == n*2);

    BOOST_TEST(shifter.getStartDelay() == 0); // offline mode
    
    size_t got = shifter.retrieve(&outp, n*2);
    BOOST_TEST(got == n*2);
    BOOST_TEST(shifter.available() == -1);

    int period = -1;
    for (int i = 1000; i < 2000; ++i) {
        if (period >= 0) ++period;
        if (out[i] <= 0.f && out[i+1] > 0.f) {
            if (period == -1) period = 0;
            else break;
        }
    }
    BOOST_TEST(period == 100);
    
    int offset = 0;
    for (int i = 0; i < 200; ++i) {
        if (out[i] <= 0.f && out[i+1] > -0.01f) {
            offset = i + 1;
            break;
        }
    }

    // overall
    
    double rms = 0.0;
    for (int i = 0; i < n - offset; ++i) {
        double diff = out[i + offset] - in[i];
        rms += diff * diff;
    }
    rms = sqrt(rms / double(n - offset));
    BOOST_TEST(rms < 0.2);

    // steady state
    
    rms = 0.0;
    for (int i = 1500; i < n - offset - 3000; ++i) {
        double diff = out[i + offset + 1500] - in[i + 1500];
        rms += diff * diff;
    }
    rms = sqrt(rms / double(n - offset - 3000));
    BOOST_TEST(rms < 0.1);
}

static vector<float> process_realtime(RubberBandLiveShifter &shifter,
                                      const vector<float> &in,
                                      int nOut,
                                      int bs,
                                      bool printDebug)
{
    int n = in.size();
    vector<float> out(nOut, 0.f);

    // Prime the start
    {
        float *source = out.data(); // just reuse out because it's silent
        shifter.process(&source, shifter.getPreferredStartPad(), false);
    }

    int toSkip = shifter.getStartDelay();
    
    int inOffset = 0, outOffset = 0;

    while (outOffset < nOut) {

        // Obtain a single block of size bs, simulating realtime
        // playback. The following might be the content of a
        // sound-producing callback function

        int needed = std::min(bs, nOut - outOffset);
        int obtained = 0;

        while (obtained < needed) {

            int available = shifter.available();

            if (available < 0) { // finished
                for (int i = obtained; i < needed; ++i) {
                    out[outOffset++] = 0.f;
                }
                break;

            } else if (available == 0) { // need to provide more input
                int required = shifter.getSamplesRequired();
                BOOST_TEST(required > 0); // because available == 0
                int toProcess = std::min(required, n - inOffset);
                const float *const source = in.data() + inOffset;
                bool final = (toProcess < required);
                shifter.process(&source, toProcess, final);
                inOffset += toProcess;
                BOOST_TEST(shifter.available() > 0);
                continue;

            } else if (toSkip > 0) { // available > 0 && toSkip > 0
                float *target = out.data() + outOffset;
                int toRetrieve = std::min(toSkip, available);
                int retrieved = shifter.retrieve(&target, toRetrieve);
                BOOST_TEST(retrieved == toRetrieve);
                toSkip -= retrieved;
                
            } else { // available > 0
                float *target = out.data() + outOffset;
                int toRetrieve = std::min(needed - obtained, available);
                int retrieved = shifter.retrieve(&target, toRetrieve);
                BOOST_TEST(retrieved == toRetrieve);
                obtained += retrieved;
                outOffset += retrieved;
            }
        }
    }

    if (printDebug) {
        // The initial # is to allow grep on the test output
        std::cout << "#sample\tV" << std::endl;
        for (int i = 0; i < nOut; ++i) {
            std::cout << "#" << i << "\t" << out[i] << std::endl;
        }
    }

    return out;
}

static void sinusoid_realtime(RubberBandLiveShifter::Options options,
                              double timeRatio,
                              double pitchScale,
                              bool printDebug)
{
    int n = (timeRatio < 1.0 ? 80000 : 40000);
    int nOut = int(ceil(n * timeRatio));
    float freq = 441.f;
    int rate = 44100;
    int bs = 512;

    // This test simulates block-by-block realtime processing with
    // latency compensation, and checks that the output is all in the
    // expected place

    RubberBandLiveShifter shifter(rate, 1, options, timeRatio, pitchScale);
    shifter.setMaxProcessSize(bs);

    if (printDebug) {
        shifter.setDebugLevel(2);
    }
    
    // The input signal is a fixed frequency sinusoid that steps up in
    // amplitude every 1/10 of the total duration - from 0.1 at the
    // start, via increments of 0.1, to 1.0 at the end
    
    vector<float> in(n);
    for (int i = 0; i < n; ++i) {
        float amplitude = float((i / (n/10)) + 1) / 10.f;
        float sample = amplitude *
            sinf(float(i) * freq * M_PI * 2.f / float(rate));
        in[i] = sample;
    }

    vector<float> out = process_realtime(shifter, in, nOut, bs, printDebug);
        
    // Step through the output signal in chunk of 1/20 of its duration
    // (i.e. a rather arbitrary two per expected 0.1 increment in
    // amplitude) and for each chunk, verify that the frequency is
    // right and the amplitude is what we expect at that point

    for (int chunk = 0; chunk < 20; ++chunk) {

//        cerr << "chunk " << chunk << " of 20" << endl;
        
        int i0 = (nOut * chunk) / 20;
        int i1 = (nOut * (chunk + 1)) / 20;

        // frequency

        int positiveCrossings = 0;
        for (int i = i0; i + 1 < i1; ++i) {
            if (out[i] <= 0.f && out[i+1] > 0.f) {
                ++positiveCrossings;
            }
        }

        int expectedCrossings = int(round((freq * pitchScale *
                                           double(i1 - i0)) / rate));

        bool highSpeedPitch =
            ! ((options & RubberBandLiveShifter::OptionPitchHighQuality) ||
               (options & RubberBandLiveShifter::OptionPitchHighConsistency));

        // The check here has to depend on whether we are in Finer or
        // Faster mode. In Finer mode, we expect to be generally exact
        // but in the first and last chunks we can be out by one
        // crossing if slowing, more if speeding up. In Faster mode we
        // need to cut more slack

        int slack = 0;

        if (options & RubberBandLiveShifter::OptionEngineFiner) {
            if (options & RubberBandLiveShifter::OptionWindowShort) {
                slack = 2;
            } else if (chunk == 0 || chunk == 19 || highSpeedPitch) {
                slack = 1;
            }
        } else {
            slack = 1;
            if (chunk == 0) {
                slack = (timeRatio < 1.0 ? 3 : 2);
            } else if (chunk == 19) {
                // all bets are off, practically
                slack = expectedCrossings / 2;
            } else {
                slack = 1;
            }
        }

        BOOST_TEST(positiveCrossings <= expectedCrossings + slack);
        BOOST_TEST(positiveCrossings >= expectedCrossings - slack);
        
        // amplitude
        
        double rms = 0.0;
        for (int i = i0; i < i1; ++i) {
            rms += out[i] * out[i];
        }
        rms = sqrt(rms / double(i1 - i0));

        double expected = (chunk/2 + 1) * 0.05 * sqrt(2.0);

        double maxOver = 0.01;
        double maxUnder = 0.1;

        if (!(options & RubberBandLiveShifter::OptionEngineFiner) ||
            (options & RubberBandLiveShifter::OptionWindowShort)) {
            maxUnder = 0.2;
        }
        
        BOOST_TEST(rms - expected < maxOver);
        BOOST_TEST(expected - rms < maxUnder);
    }        
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_samepitch_realtime_finer)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      8.0, 1.0,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_samepitch_realtime_finer)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      0.5, 1.0,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_higher_realtime_finer)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_higher_realtime_finer_hqpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighQuality,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_higher_realtime_finer_hcpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_higher_realtime_finer)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      0.5, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_higher_realtime_finer_hqpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighQuality,
                      0.5, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_higher_realtime_finer_hcpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      0.5, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_lower_realtime_finer)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_lower_realtime_finer_hqpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighQuality,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_lower_realtime_finer_hcpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_samepitch_realtime_finer_short)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionWindowShort,
                      8.0, 1.0,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_samepitch_realtime_finer_short)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionWindowShort,
                      0.5, 1.0,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_higher_realtime_finer_short)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionWindowShort,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_higher_realtime_finer_short_hcpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionWindowShort |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_higher_realtime_finer_short)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionWindowShort,
                      0.5, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_higher_realtime_finer_hcpitch_short)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionWindowShort |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      0.5, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_lower_realtime_finer_short)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionWindowShort,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_lower_realtime_finer_short_hcpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionWindowShort |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_samepitch_realtime_faster)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      8.0, 1.0,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_samepitch_realtime_faster)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      0.5, 1.0,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_higher_realtime_faster)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_higher_realtime_faster_hqpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighQuality,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_higher_realtime_faster_hcpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_higher_realtime_faster)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      0.5, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_higher_realtime_faster_hqpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighQuality,
                      0.5, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_fast_higher_realtime_faster_hcpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      0.5, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_lower_realtime_faster)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_lower_realtime_faster_hqpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighQuality,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(sinusoid_slow_lower_realtime_faster_hcpitch)
{
    sinusoid_realtime(RubberBandLiveShifter::OptionEngineFaster |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(impulses_2x_offline_faster)
{
    int n = 10000;
    int rate = 44100;
    RubberBandLiveShifter shifter
        (rate, 1, RubberBandLiveShifter::OptionEngineFaster);

    shifter.setTimeRatio(2.0);

    vector<float> in(n, 0.f), out(n * 2, 0.f);

    in[100] = 1.f;
    in[101] = -1.f;

    in[5000] = 1.f;
    in[5001] = -1.f;

    in[9900] = 1.f;
    in[9901] = -1.f;
    
    float *inp = in.data(), *outp = out.data();

    shifter.setMaxProcessSize(n);
    shifter.setExpectedInputDuration(n);
    BOOST_TEST(shifter.available() == 0);

    shifter.study(&inp, n, true);
    BOOST_TEST(shifter.available() == 0);

    shifter.process(&inp, n, true);
    BOOST_TEST(shifter.available() == n * 2);

    BOOST_TEST(shifter.getStartDelay() == 0); // offline mode
    
    size_t got = shifter.retrieve(&outp, n * 2);
    BOOST_TEST(got == n * 2);
    BOOST_TEST(shifter.available() == -1);

    int peak0 = -1, peak1 = -1, peak2 = -1;
    float max;
    
    max = -2.f;
    for (int i = 0; i < n/2; ++i) {
        if (out[i] > max) { max = out[i]; peak0 = i; }
    }

    max = -2.f;
    for (int i = n/2; i < (n*3)/2; ++i) {
        if (out[i] > max) { max = out[i]; peak1 = i; }
    }

    max = -2.f;
    for (int i = (n*3)/2; i < n*2; ++i) {
        if (out[i] > max) { max = out[i]; peak2 = i; }
    }

    BOOST_TEST(peak0 == 100);
    BOOST_TEST(peak1 > n - 400);
    BOOST_TEST(peak1 < n + 50);
    BOOST_TEST(peak2 > n*2 - 600);
    BOOST_TEST(peak2 < n*2);
/*
    std::cout << "ms\tV" << std::endl;
    for (int i = 0; i < n*2; ++i) {
        std::cout << i << "\t" << out[i] << std::endl;
    }
*/
}

BOOST_AUTO_TEST_CASE(impulses_2x_offline_finer)
{
    int n = 10000;
    int rate = 44100;
    RubberBandLiveShifter shifter
        (rate, 1, RubberBandLiveShifter::OptionEngineFiner);

    shifter.setTimeRatio(2.0);

    vector<float> in(n, 0.f), out(n * 2, 0.f);

    in[100] = 1.f;
    in[101] = -1.f;

    in[5000] = 1.f;
    in[5001] = -1.f;

    in[9900] = 1.f;
    in[9901] = -1.f;
    
    float *inp = in.data(), *outp = out.data();

    shifter.setMaxProcessSize(n);
    shifter.setExpectedInputDuration(n);
    BOOST_TEST(shifter.available() == 0);

    shifter.study(&inp, n, true);
    BOOST_TEST(shifter.available() == 0);

    shifter.process(&inp, n, true);
    BOOST_TEST(shifter.available() == n * 2);

    BOOST_TEST(shifter.getStartDelay() == 0); // offline mode
    
    size_t got = shifter.retrieve(&outp, n * 2);
    BOOST_TEST(got == n * 2);
    BOOST_TEST(shifter.available() == -1);

    int peak0 = -1, peak1 = -1, peak2 = -1;
    float max;

    max = -2.f;
    for (int i = 0; i < n/2; ++i) {
        if (out[i] > max) { max = out[i]; peak0 = i; }
    }

    max = -2.f;
    for (int i = n/2; i < (n*3)/2; ++i) {
        if (out[i] > max) { max = out[i]; peak1 = i; }
    }

    max = -2.f;
    for (int i = (n*3)/2; i < n*2; ++i) {
        if (out[i] > max) { max = out[i]; peak2 = i; }
    }

    BOOST_TEST(peak0 == 100);
    BOOST_TEST(peak1 > n - 400);
    BOOST_TEST(peak1 < n + 50);
    BOOST_TEST(peak2 > n*2 - 600);
    BOOST_TEST(peak2 < n*2);
/*
    std::cout << "ms\tV" << std::endl;
    for (int i = 0; i < n*2; ++i) {
        std::cout << i << "\t" << out[i] << std::endl;
    }
*/
}

BOOST_AUTO_TEST_CASE(impulses_2x_5up_offline_finer)
{
    int n = 10000;
    int rate = 44100;
    RubberBandLiveShifter shifter
        (rate, 1, RubberBandLiveShifter::OptionEngineFiner);

    shifter.setTimeRatio(2.0);
    shifter.setPitchScale(1.5);

    vector<float> in(n, 0.f), out(n * 2, 0.f);

    in[100] = 1.f;
    in[101] = -1.f;

    in[5000] = 1.f;
    in[5001] = -1.f;

    in[9900] = 1.f;
    in[9901] = -1.f;
    
    float *inp = in.data(), *outp = out.data();

    shifter.setMaxProcessSize(n);
    shifter.setExpectedInputDuration(n);
    BOOST_TEST(shifter.available() == 0);

    shifter.study(&inp, n, true);
    BOOST_TEST(shifter.available() == 0);

    shifter.process(&inp, n, true);
    BOOST_TEST(shifter.available() == n * 2);

    BOOST_TEST(shifter.getStartDelay() == 0); // offline mode
    
    size_t got = shifter.retrieve(&outp, n * 2);
    BOOST_TEST(got == n * 2);
    BOOST_TEST(shifter.available() == -1);

    int peak0 = -1, peak1 = -1, peak2 = -1;
    float max;

    max = -2.f;
    for (int i = 0; i < n/2; ++i) {
        if (out[i] > max) { max = out[i]; peak0 = i; }
    }

    max = -2.f;
    for (int i = n/2; i < (n*3)/2; ++i) {
        if (out[i] > max) { max = out[i]; peak1 = i; }
    }

    max = -2.f;
    for (int i = (n*3)/2; i < n*2; ++i) {
        if (out[i] > max) { max = out[i]; peak2 = i; }
    }

    BOOST_TEST(peak0 < 100);
    BOOST_TEST(peak1 > n - 400);
    BOOST_TEST(peak1 < n + 50);
    BOOST_TEST(peak2 > n*2 - 600);
    BOOST_TEST(peak2 < n*2);
/*
    std::cout << "ms\tV" << std::endl;
    for (int i = 0; i < n*2; ++i) {
        std::cout << i << "\t" << out[i] << std::endl;
    }
*/
}

static void impulses_realtime(RubberBandLiveShifter::Options options,
                              double timeRatio,
                              double pitchScale,
                              bool printDebug)
{
    int n = 10000;
    int nOut = int(ceil(n * timeRatio));
    int rate = 48000;
    int bs = 1024;

    RubberBandLiveShifter shifter(rate, 1, options, timeRatio, pitchScale);

    if (printDebug) {
        shifter.setDebugLevel(2);
    }
    
    vector<float> in(n, 0.f);

    in[100] = 1.f;
    in[101] = -1.f;

    in[5000] = 1.f;
    in[5001] = -1.f;

    in[9900] = 1.f;
    in[9901] = -1.f;

    vector<float> out = process_realtime(shifter, in, nOut, bs, printDebug);

    int peak0 = -1, peak1 = -1, peak2 = -1;
    float max;

    max = -2.f;
    for (int i = 0; i < nOut/4; ++i) {
        if (out[i] > max) { max = out[i]; peak0 = i; }
    }

    max = -2.f;
    for (int i = nOut/4; i < (nOut*3)/4; ++i) {
        if (out[i] > max) { max = out[i]; peak1 = i; }
    }

    max = -2.f;
    for (int i = (nOut*3)/4; i < nOut; ++i) {
        if (out[i] > max) { max = out[i]; peak2 = i; }
    }

    // These limits aren't alarming, but it be worth tightening them
    // and and taking a look at the waveforms
    
    BOOST_TEST(peak0 < int(ceil(200 * timeRatio)));
    BOOST_TEST(peak0 > int(ceil(50 * timeRatio)));

    BOOST_TEST(peak1 < int(ceil(5070 * timeRatio)));
    BOOST_TEST(peak1 > int(ceil(4840 * timeRatio)));

    BOOST_TEST(peak2 < int(ceil(9970 * timeRatio)));
    BOOST_TEST(peak2 > int(ceil(9770 * timeRatio)));

/*
    std::cout << "ms\tV" << std::endl;
    for (int i = 0; i < n*2; ++i) {
        std::cout << i << "\t" << out[i] << std::endl;
    }
*/
}

BOOST_AUTO_TEST_CASE(impulses_slow_samepitch_realtime_finer)
{
    impulses_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      8.0, 1.0,
                      false);
}

BOOST_AUTO_TEST_CASE(impulses_fast_samepitch_realtime_finer)
{
    impulses_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      0.5, 1.0,
                      false);
}

BOOST_AUTO_TEST_CASE(impulses_slow_higher_realtime_finer)
{
    impulses_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(impulses_slow_higher_realtime_finer_hqpitch)
{
    impulses_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighQuality,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(impulses_slow_higher_realtime_finer_hcpitch)
{
    impulses_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      4.0, 1.5,
                      false);
}

BOOST_AUTO_TEST_CASE(impulses_slow_lower_realtime_finer)
{
    impulses_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(impulses_slow_lower_realtime_finer_hqpitch)
{
    impulses_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighQuality,
                      8.0, 0.5,
                      false);
}

BOOST_AUTO_TEST_CASE(impulses_slow_lower_realtime_finer_hcpitch)
{
    impulses_realtime(RubberBandLiveShifter::OptionEngineFiner |
                      RubberBandLiveShifter::OptionProcessRealTime |
                      RubberBandLiveShifter::OptionPitchHighConsistency,
                      8.0, 0.5,
                      false);
}

static void final_realtime(RubberBandLiveShifter::Options options,
                           double timeRatio,
                           double pitchScale,
                           bool finalAfterFinishing,
                           bool printDebug)
{
    int n = 10000;
    float freq = 440.f;
    int rate = 44100;
    int blocksize = 700;
    RubberBandLiveShifter shifter(rate, 1, options);

    if (printDebug) {
        shifter.setDebugLevel(2);
    }

    shifter.setTimeRatio(timeRatio);
    shifter.setPitchScale(pitchScale);

    int nOut = int(ceil(n * timeRatio));
    int excess = std::max(nOut, n);
    vector<float> in(n, 0.f), out(nOut + excess, 0.f);

    for (int i = 0; i < 100; ++i) {
        in[n - 101 + i] = sinf(float(i) * freq * M_PI * 2.f / float(rate));
    }
    
    // Prime the start
    {
        float *source = out.data(); // just reuse out because it's silent
        shifter.process(&source, shifter.getPreferredStartPad(), false);
    }

    float *inp = in.data(), *outp = out.data();

    shifter.setMaxProcessSize(blocksize);
    BOOST_TEST(shifter.available() == 0);

    int toSkip = shifter.getStartDelay();

    int incount = 0, outcount = 0;
    while (true) {

        int inbs = std::min(blocksize, n - incount);

        bool final;
        if (finalAfterFinishing) {
            BOOST_TEST(inbs >= 0);
            final = (inbs == 0);
        } else {
            BOOST_TEST(inbs > 0);
            final = (incount + inbs >= n);
        }
        
        float *in = inp + incount;
        shifter.process(&in, inbs, final);
        incount += inbs;

        int avail = shifter.available();
        BOOST_TEST(avail >= 0);
        BOOST_TEST(outcount + avail < nOut + excess);

//        cerr << "in = " << inbs << ", incount now = " << incount << ", avail = " << avail << endl;

        float *out = outp + outcount;

        if (toSkip > 0) {
            int skipHere = std::min(toSkip, avail);
            size_t got = shifter.retrieve(&out, skipHere);
            BOOST_TEST(got == size_t(skipHere));
            toSkip -= got;
//            cerr << "got = " << got << ", toSkip now = " << toSkip << ", n = " << n << endl;
        }

        avail = shifter.available();
        if (toSkip == 0 && avail > 0) {
            size_t got = shifter.retrieve(&out, avail);
            BOOST_TEST(got == size_t(avail));
            outcount += got;
//            cerr << "got = " << got << ", outcount = " << outcount << ", n = " << n << endl;
            if (final) {
                BOOST_TEST(shifter.available() == -1);
            } else {
                BOOST_TEST(shifter.available() == 0);
            }
        }

        if (final) break;
    }

    BOOST_TEST(outcount >= nOut);
    
    if (printDebug) {
        // The initial # is to allow grep on the test output
        std::cout << "#sample\tV" << std::endl;
        for (int i = 0; i < outcount; ++i) {
            std::cout << "#" << i << "\t" << out[i] << std::endl;
        }
    }

}

BOOST_AUTO_TEST_CASE(final_slow_samepitch_realtime_finer)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   8.0, 1.0,
                   false,
                   false);
}

BOOST_AUTO_TEST_CASE(final_slow_samepitch_realtime_finer_after)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   8.0, 1.0,
                   true,
                   false);
}

BOOST_AUTO_TEST_CASE(final_fast_samepitch_realtime_finer)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   0.2, 1.0,
                   false,
                   false);
}

BOOST_AUTO_TEST_CASE(final_fast_samepitch_realtime_finer_after)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   0.2, 1.0,
                   true,
                   false);
}

BOOST_AUTO_TEST_CASE(final_slow_higher_realtime_finer)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   8.0, 1.5,
                   false,
                   false);
}

BOOST_AUTO_TEST_CASE(final_slow_higher_realtime_finer_after)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   8.0, 1.5,
                   true,
                   false);
}

BOOST_AUTO_TEST_CASE(final_fast_higher_realtime_finer)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   0.2, 1.5,
                   false,
                   false);
}

BOOST_AUTO_TEST_CASE(final_fast_higher_realtime_finer_after)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   0.2, 1.5,
                   true,
                   false);
}

BOOST_AUTO_TEST_CASE(final_slow_lower_realtime_finer)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   8.0, 0.5,
                   false,
                   false);
}

BOOST_AUTO_TEST_CASE(final_slow_lower_realtime_finer_after)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   8.0, 0.5,
                   true,
                   false);
}

BOOST_AUTO_TEST_CASE(final_fast_lower_realtime_finer)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   0.2, 0.5,
                   false,
                   false);
}

BOOST_AUTO_TEST_CASE(final_fast_lower_realtime_finer_after)
{
    final_realtime(RubberBandLiveShifter::OptionEngineFiner |
                   RubberBandLiveShifter::OptionProcessRealTime |
                   RubberBandLiveShifter::OptionPitchHighConsistency,
                   0.2, 0.5,
                   true,
                   false);
}
#endif // NOT_DEFINED

BOOST_AUTO_TEST_SUITE_END()
