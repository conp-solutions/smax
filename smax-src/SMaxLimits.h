/************************************************************************************[SMaxLimits.h]

Copyright (c) 2017-2018, Norbert Manthey, all rights reserved.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **************************************************************************************************/

#ifndef SMaxLimits_h
#define SMaxLimits_h

/*
 *  This file implements sets the limits that are used to reject a formula in smoother
 */

// largest variable should be below 16M
#define SMAX_MAX_VAR (1 << 24)

// highest number of clauses, 64M
#define SMAX_MAX_CLS (1 << 26)

// do not allow weights greater than this value
#define SMOOTHER_MAX_WEIGHT (1ULL << 61)

#endif
