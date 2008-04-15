/************************* MPEG-4 AAC Audio Decoder **************************
 *                                                                           *
"This software module was originally developed by
AT&T, Dolby Laboratories, Fraunhofer Gesellschaft IIS in the course of
development of the MPEG-2 AAC/MPEG-4 Audio standard ISO/IEC 13818-7,
14496-1,2 and 3. This software module is an implementation of a part of one or more
MPEG-2 AAC/MPEG-4 Audio tools as specified by the MPEG-2 AAC/MPEG-4
Audio standard. ISO/IEC  gives users of the MPEG-2 AAC/MPEG-4 Audio
standards free license to this software module or modifications thereof for use in
hardware or software products claiming conformance to the MPEG-2 AAC/MPEG-4
Audio  standards. Those intending to use this software module in hardware or
software products are advised that this use may infringe existing patents.
The original developer of this software module and his/her company, the subsequent
editors and their companies, and ISO/IEC have no liability for use of this software
module or modifications thereof in an implementation. Copyright is not released for
non MPEG-2 AAC/MPEG-4 Audio conforming products. The original developer
retains full right to use the code for his/her  own purpose, assign or donate the
code to a third party and to inhibit third party from using the code for non
MPEG-2 AAC/MPEG-4 Audio conforming products. This copyright notice must
be included in all copies or derivative works."
Copyright(c)1996.
 *                                                                           *
 ****************************************************************************/
/*
 * $Id: pns.c,v 1.10 2002/01/04 13:34:44 menno Exp $
 */

#include "all.h"


#define MEAN_NRG 1.537228e+18      /* Theory: (2^31)^2 / 3 = 1.537228e+18 */

static long random2(long *seed)
{
    *seed = (1664525L * *seed) + 1013904223L;  /* Numerical recipes */

    return *seed;
}

static void gen_rand_vector(float *spec, int size, long *state)
{
    int i;

    for (i = 0; i < size; i++)
    {
        spec[i] = (float)random2(state);
    }
}



/*
 * if (noise correlated) {
 *   restore saved left channel random generator state
 *   generate random values
 * } else {
 *   save current random generator state
 *   generate random values
 * }
 * scale according to scalefactor
 *
 * Important: needs to be called left channel, then right channel
 *            for each channel pair
 */

void pns(faacDecHandle hDecoder, MC_Info *mip, Info *info, int widx, int ch,
         byte *group, byte *cb_map, int *factors,
         int *lpflag, Float *coef[Chans] )
{
    Ch_Info *cip = &mip->ch_info[ch];
    Float   *spec, *fp, scale;
    int     cb, sfb, n, nn, b, bb, nband;
    int   *band;
//ms999    long    *nsp;
    long    *nsp=0;

    static int state = 1;

    /* store original predictor flags when left channel of a channel pair */
    if ((cip->cpe  &&  cip->ch_is_left  &&  info->islong))
        for (sfb=0; sfb<info->sfb_per_sbk[0]; sfb++)
            hDecoder->lp_store[sfb+1] = lpflag[sfb+1];

    /* restore original predictor flags when right channel of a channel pair */
    if ((cip->cpe  &&  !cip->ch_is_left  &&  info->islong))
        for (sfb=0; sfb<info->sfb_per_sbk[0]; sfb++)
            lpflag[sfb+1] = hDecoder->lp_store[sfb+1];

    spec = coef[ ch ];

    /* PNS goes by group */
    bb = 0;
    for (b = 0; b < info->nsbk; ) {
        nband = info->sfb_per_sbk[b];
        band = info->sbk_sfb_top[b];

        b = *group++;       /* b = index of last sbk in group */
        for (; bb < b; bb++) {  /* bb = sbk index */
            n = 0;
            for (sfb = 0; sfb < nband; sfb++){
                nn = band[sfb]; /* band is offset table, nn is last coef in band */
                cb = cb_map[sfb];
                if (cb == NOISE_HCB  ||  cb == NOISE_HCB+100) {
                    /* found noise  substitution code book */

                    /* disable prediction (only important for long blocks) */
                    if (info->islong)  lpflag[1+sfb] = 0;

                    /* reconstruct noise substituted values */
                    /* generate random noise */
                    fp = spec + n;
                    gen_rand_vector(fp, nn-n, &state);

                    /* 14496-3 says:
                       scale = 1.0f/(size * (float)sqrt(MEAN_NRG));
                     */
                    scale = 1.0f/(float)sqrt((nn-n) * MEAN_NRG);
                    scale *= (float)pow(2.0, 0.25*factors[sfb]);

                    /* Scale random vector to desired target energy */
                    for (; n < nn; n++)
                        *fp++ *= scale;
                }
                n = nn;
            }
            spec += info->bins_per_sbk[bb];
            factors += nband;
        }
        nsp += info->sfb_per_sbk[bb-1];
        cb_map += info->sfb_per_sbk[bb-1];
    }
}


/********************************************************************************
 *** FUNCTION: predict_pns_reset()                                              *
 ***                                        *
 ***    carry out predictor reset for PNS scalefactor bands (long blocks)       *
 ***                                        *
 ********************************************************************************/
void predict_pns_reset(Info* info, PRED_STATUS *psp, byte *cb_map)
{
    int    nband, sfb, i, top;
    int    *band;

    if (info->islong) {

        nband = info->sfb_per_sbk[0];
        band = info->sbk_sfb_top[0];

        for (i=0,sfb=0; sfb<nband; sfb++)  {

            top = band[sfb];
            if (cb_map[sfb] == NOISE_HCB  ||  cb_map[sfb] == NOISE_HCB+100) {
                for (; i<top; i++)
                    reset_pred_state(&psp[i]);
            }
            i = top;
        }

    } /* end islong */
}

