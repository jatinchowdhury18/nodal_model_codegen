// Auto-generated with netlist_codegen version ce593e9.
// Command: netlist_codegen rc_lowpass.net rc_lowpass_c.h -lang c

#pragma once

typedef struct {
    float R1; // = 1.0e+03
    float C1; // = 1.0e-06
} Params;

typedef struct {
    float zC1;
} State;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const float gR1 = 1.0f / params.R1;
    
    const float gC1 = 2.0f * sample_rate * params.C1;
    
    const float _t0 = (1.0f / (gR1 + gC1));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        float zC1 = state[ch].zC1;
        for (int n = 0; n < num_samples; ++n)
        {
            const float vi = input[ch][n];

            const float vo = (((gR1 * vi) + zC1) * _t0);
            const float tC1 = (gC1 * (vo - 0));
            
            zC1 = 2 * tC1 - zC1;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const float gR1 = 1.0f / params.R1;
    
    const float gC1 = 2.0f * sample_rate * params.C1;
    
    const float vi = vi_dc;

    const float zC1 = ((gC1 * (gR1 * vi)) / (gR1 + (1.0f / 1000000000.0f)));

    const float vo_dc_out = ((gR1 * vi) / (gR1 + (1.0f / 1000000000.0f)));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zC1 = zC1;
    }
    return vo_dc_out;
}

