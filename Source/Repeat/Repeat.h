/*
  ==============================================================================

    Repeat.h
    Created: 12 Mar 2021 5:52:55pm
    Author:  migizo

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AnimatorByCount.h"
#include <cmath>
class Repeat {
public:
    void setInterval(int _interval) {
		// Interval is the how long we want to repeat the samples, typically expressed by note length
		// e.g. 8th, 12th, 16th, etc.
		// if we pass 0 to it, that means this effect is bypassed
       if (preserveInterval != _interval) {
            if (this->interval == 0) {
                clear();
                enablePush = true;  // starts to get samples once the interval changes from 0 to something else
            }
            preserveInterval = _interval;
            
			
			// fade animator is started when interval value changes
            animator.start(100, 1.0, 0.0);
        }
    }
    
    //------------------------------------------------------------------------
    void setParameter(double sampleRate, double bpm) {
        
        // set the buffer parameter when bpm changes
        maxBufferSize = sampleRate * 60.0 / 1.0/*bpm*/ * 4.0; // bpm=1
        bufferSize = sampleRate * (60.0 / bpm * 4.0); // the sample count of a 4-quarter note measure

		buffer.reserve(2 * maxBufferSize); // We want to prevent re-allocation in audio thread
		// Still... using std::vector is a pretty bad idea and this fx will occasionally fail.
		// You should really use AudioSampleBuffer provided by juce if you want to recreate this effect.
    }
    
    //------------------------------------------------------------------------
    template <typename T>
    void process(T & val) {
//        val *= sin((double)count/(1000.0*(double)(interval+1)));
//        count++;
//        return;
        
        // write buffer. The input val here is typically one sample in the streaming audio buffer
        if (enablePush) {
            // if the buffer is not full, push it in buffer
            if (buffer.size() < maxBufferSize) {
                buffer.push_back(val);
            }
            // otherwise we stop pushing
            if (buffer.size() == maxBufferSize) enablePush = false;
        }
        
        if (interval != 0) {
            // read the buffer and write buffered value to val, thus we play some previous samples repeatedly
            int divSize = (double)bufferSize / (double)interval;  // how many samples in each repeat interval
            int divCount = count % divSize;  // count how many samples passed in the outer loop (in RepeatProcessor.h) and get an index
            const int fadeSize = 100;
            val = buffer[divCount];  // write to the streaming buffer
            
            // Fade in, at the head of each repeated interval
            if (divCount < fadeSize) {
                val *= juce::jmap((double)(divCount), 0.0, (double)(fadeSize - 1), 0.0, 1.0);
            }
            // Fade out at tail, similarly
            else if (divCount > (divSize - fadeSize)) {
                val *= juce::jmap((double)(divCount), (double)(divSize - fadeSize), (double)(divSize - 1), 1.0, 0.0);
            }
        }
        
        // Handle fade-out in the process of changing the interval parameter (see setInterval function)
		// see AnimatorByCount.h
        animator.update();
        if (animator.getIsProgress()) {
            double v = animator.getValue();
            val *= v;
            // fadeout complete -> update interval
            if (v <= 0.0 && animator.getNormalizedValue() >= 1.0) {
                animator.start(100, 0.0, 1.0);
                interval = preserveInterval;
            }
        }
        
        count++;
    }
    
    //------------------------------------------------------------------------
    void clear() {
        count = 0;
        buffer.clear();
    }
    
    //------------------------------------------------------------------------

private:
    int count = 0;
    std::vector<double> buffer;
    AnimatorByCount animator;
    
    int maxBufferSize;
    int bufferSize;
    bool enablePush = false;
    int interval = 0;
    int preserveInterval = 0;

};
