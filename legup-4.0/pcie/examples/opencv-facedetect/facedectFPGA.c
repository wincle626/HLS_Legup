
#include <stdio.h>
// #include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/types_c.h>
#define DEBUG 0
#define OF_CHECK 0

#include "fixedptc.h"
#include <pthread.h>
#include "legup_mem.h"

#define CV_HAAR_FEATURE_MAX 3
typedef int sumtype;
typedef double sqsumtype;

typedef struct
{
    sumtype *p0, *p1, *p2, *p3;
    fixedpt weight;
    // float weight;
} HaarRect;

typedef struct CvHidHaarFeature
{
    HaarRect rect[CV_HAAR_FEATURE_MAX];
} CvHidHaarFeature;


typedef struct CvHidHaarTreeNode
{
    CvHidHaarFeature feature;
    fixedpt threshold;
    // float threshold;
    int left;
    int right;
} CvHidHaarTreeNode;


typedef struct CvHidHaarClassifier
{
    int count;
    //CvHaarFeature* orig_feature;
    CvHidHaarTreeNode* node;
    fixedpt* alpha;
} CvHidHaarClassifier;


typedef struct CvHidHaarStageClassifier
{
    int  count;
    fixedpt threshold;
    // float threshold;
    CvHidHaarClassifier* classifier;
    int two_rects;

    struct CvHidHaarStageClassifier* next;
    struct CvHidHaarStageClassifier* child;
    struct CvHidHaarStageClassifier* parent;
} CvHidHaarStageClassifier;


typedef struct CvHidHaarClassifierCascade
{
    int  count;
    int  isStumpBased;
    int  has_tilted_features;
    int  is_tree;
    double inv_window_area;
    CvMat sum, sqsum, tilted;
    CvHidHaarStageClassifier* stage_classifier;
    sqsumtype *pq0, *pq1, *pq2, *pq3;
    sumtype *p0, *p1, *p2, *p3;

    void** ipp_stages;
} CvHidHaarClassifierCascade;

#define GETPTR(base_new, ptr_old, base_old) ((char *)(base_new) + ((char *)(ptr_old) - (char *)(base_old)))

#define update_base(ptr_old) GETPTR(cascade_fpga_base, ptr_old, cascade)
#define update_sum(ptr_old) ((sumtype *)(GETPTR(sum_fpga, ptr_old, old_sum_ptr)))
#define update_sqsum(ptr_old) ((sqsumtype *)(GETPTR(sqsum_fpga, ptr_old, old_sqsum_ptr)))
#define get_ptr(ptr) GETPTR(cascade_fpga_image, ptr, cascade_fpga_base)

#define ISWITHINRANGE(ptr, base, range) ((char *)(ptr) - (char *)(base) >= 0 && \
        (char *)(ptr) - (char *)(base) < range)
#define ISWITHINRANGEOLD(ptr) ISWITHINRANGE(ptr, cascade, datasize)

#if DEBUG == 1
#define ASSERT_DOUBLE_EQ(value1, value2) assert(FABS((value1) - (value2)) < 0.01)
#define assert_size(ptr, base, size) assert(ISWITHINRANGE(ptr, base, size))
#else
#define ASSERT_DOUBLE_EQ(value1, value2) ;
#define assert_size(ptr, base, size) ;
#endif

#define assert_old(ptr) assert_size(ptr, cascade, datasize)
#define assert_sum(ptr) assert_size(ptr, old_sum_ptr, sumsize)
#define assert_new(ptr) assert_size(ptr, cascade_fpga_base, datasize)

#define GET_VALUE(TYPE, V) (*(TYPE *)(&(V)))
#define GET_FLOAT(V) GET_VALUE(float, V)
#define CVT_FLOAT_TO_FIXEDPT(V) fixedpt_fromfloat(GET_FLOAT(V))

#define ROUNDUP16(val) (((val) + 0x10) & 0xfffffff0)

// #define malloc_shared(size, oldptr, loc) malloc(size)
// #define free_shared free
// #define memcpy_to_shared memcpy

static inline int calc_sum(HaarRect rect, int offset) {
    return ((rect).p0[offset] - (rect).p1[offset] - (rect).p2[offset] + (rect).p3[offset]);
}

// static char data[1024 * 1024 * 64];
// static CvHidHaarClassifierCascade * cascade_host = NULL;
// static uchar *  sum_fpga = NULL; // [1024 * 1024 * 64];
// static uchar * sum_fpga = NULL;
// static CvMat sqsum;
// static uchar * sqsum_fpga = NULL;

void* copySumToFPGA(CvMat * _sum) {
    size_t size = ROUNDUP16(_sum->rows * _sum->cols * sizeof(sumtype));
    void * sum_fpga = malloc_shared(size, NULL, LEGUP_RAM_LOCATION_DDR2);
    memcpy_to_shared( sum_fpga, _sum->data.ptr, size);
    return sum_fpga;
}

void* copySqSumToFPGA(CvMat * _sqsum) {
    size_t size = ROUNDUP16(_sqsum->rows * _sqsum->cols * sizeof(sqsumtype));
    void *sqsum_fpga = (uchar *) malloc_shared(size, NULL, LEGUP_RAM_LOCATION_DDR2);
    memcpy_to_shared( sqsum_fpga, _sqsum->data.ptr, size);
    return sqsum_fpga;
    // sqsum.data.ptr = sqsum_fpga;
    // for (int i = 0; i < size; i++)
    //     *((fixedptd *)&sqsum.data.db[i]) = fixedpt_fromdouble(sqsum.data.db[i]);
}

void cleanUp(void * sum, void * sqsum, void * cascade)
{
    free_shared(sum);
    free_shared(sqsum);
    free_shared(cascade);
}


CvHidHaarClassifierCascade*
copyHidHaarClassifierCascadeToFPGA( CvHidHaarClassifierCascade* cascade, void * sum_fpga, void * sqsum_fpga)
{
    assert(cascade->isStumpBased);

    // CvHidHaarClassifierCascade* out = NULL;
    CvHidHaarClassifierCascade* cascade_fpga_image = NULL;
    CvHidHaarClassifierCascade* cascade_fpga_base = NULL;

    int i, j, l;
    int datasize;
    int total_classifiers = 0;
    int total_nodes = 0;
    char errorstr[1000];
    int max_count = 0;

    /* check input structure correctness and calculate total memory size needed for
       internal representation of the classifier cascade */
    for( i = 0; i < cascade->count; i++ )
    {
        CvHidHaarStageClassifier* stage_classifier = cascade->stage_classifier + i;

        if( !stage_classifier->classifier ||
            stage_classifier->count <= 0 )
        {
            sprintf( errorstr, "header of the stage classifier #%d is invalid "
                     "(has null pointers or non-positive classfier count)", i );
            exit(1);
            // CV_Error( CV_StsError, errorstr );
        }

        max_count = MAX( max_count, stage_classifier->count );
        total_classifiers += stage_classifier->count;

        for( j = 0; j < stage_classifier->count; j++ )
        {
            CvHidHaarClassifier* classifier = stage_classifier->classifier + j;

            total_nodes += classifier->count;
        }
    }

    // this is an upper boundary for the whole hidden cascade size
    datasize = sizeof(CvHidHaarClassifierCascade) +
               sizeof(CvHidHaarStageClassifier)*cascade->count +
               sizeof(CvHidHaarClassifier) * total_classifiers +
               sizeof(CvHidHaarTreeNode) * total_nodes +
               sizeof(void*)*(total_nodes + total_classifiers);

    datasize = ROUNDUP16(datasize);

    // out = (CvHidHaarClassifierCascade*)data;// malloc( datasize );
    cascade_fpga_image = (CvHidHaarClassifierCascade*) malloc( datasize );
    cascade_fpga_base  = (CvHidHaarClassifierCascade*) malloc_shared( datasize, NULL, LEGUP_RAM_LOCATION_DDR2 );
    assert(cascade_fpga_image != NULL);
    assert(cascade_fpga_base != NULL);
    memcpy( cascade_fpga_image, cascade, datasize );
    // memset(cascade_fpga_base, 0, datasize);
    // cascade_host = cascade_fpga_base;

    void * old_sum_ptr = cascade_fpga_image->sum.data.ptr;
#if DEBUG == 1
    size_t sumsize = cascade_fpga_image->sum.rows * cascade_fpga_image->sum.cols * sizeof(sumtype);
#endif
    void * old_sqsum_ptr = cascade_fpga_image->sqsum.data.ptr;

    /* init header */
    // cascade_fpga_image->count = cascade->count;
    assert_old(cascade->stage_classifier);
    cascade_fpga_image->stage_classifier =
            (CvHidHaarStageClassifier *) update_base(cascade->stage_classifier);
    assert_new(cascade_fpga_image->stage_classifier);

    cascade_fpga_image->p0 = update_sum(cascade->p0);
    cascade_fpga_image->p1 = update_sum(cascade->p1);
    cascade_fpga_image->p2 = update_sum(cascade->p2);
    cascade_fpga_image->p3 = update_sum(cascade->p3);

    cascade_fpga_image->pq0 = update_sqsum(cascade->pq0);
    cascade_fpga_image->pq1 = update_sqsum(cascade->pq1);
    cascade_fpga_image->pq2 = update_sqsum(cascade->pq2);
    cascade_fpga_image->pq3 = update_sqsum(cascade->pq3);

    /* initialize internal representation */
    for( i = 0; i < cascade->count; i++ )
    {
        CvHidHaarStageClassifier* stage_classifier = cascade->stage_classifier + i;
        CvHidHaarStageClassifier* hid_stage_classifier =
            ((CvHidHaarStageClassifier *)get_ptr(cascade_fpga_image->stage_classifier)) + i;

        hid_stage_classifier->threshold =
            CVT_FLOAT_TO_FIXEDPT(hid_stage_classifier->threshold);
        ASSERT_DOUBLE_EQ ((double)GET_FLOAT(stage_classifier->threshold), fixedpt_todouble(hid_stage_classifier->threshold));

        assert_old(stage_classifier->classifier);
        hid_stage_classifier->classifier = (CvHidHaarClassifier *)
            update_base(stage_classifier->classifier);

        assert(stage_classifier->parent == NULL || ISWITHINRANGEOLD(stage_classifier->parent));
        hid_stage_classifier->parent = stage_classifier->parent ? (CvHidHaarStageClassifier *)
            update_base(stage_classifier->parent) : NULL;

        assert(stage_classifier->next == NULL || ISWITHINRANGEOLD(stage_classifier->next));
        hid_stage_classifier->next   = stage_classifier->next ? (CvHidHaarStageClassifier *)
            update_base(stage_classifier->next) : NULL;

        assert(stage_classifier->child == NULL || ISWITHINRANGEOLD(stage_classifier->child));
        hid_stage_classifier->child  = stage_classifier->child ? (CvHidHaarStageClassifier *)
            update_base(stage_classifier->child) : NULL;

        for( j = 0; j < stage_classifier->count; j++ )
        {
            CvHidHaarClassifier* classifier = stage_classifier->classifier + j;
            CvHidHaarClassifier* hid_classifier = ((CvHidHaarClassifier *)get_ptr(hid_stage_classifier->classifier)) + j;

            int node_count = classifier->count;
            int two_rects = stage_classifier->two_rects;

            assert_old(classifier->alpha);
            assert_old(classifier->node);
            hid_classifier->alpha = (fixedpt *) update_base(classifier->alpha);
            hid_classifier->node  = (CvHidHaarTreeNode *) update_base(classifier->node);

            for (l = 0; l < node_count + 1; l++ ) {
                ((fixedpt *)get_ptr(hid_classifier->alpha))[l] =
                    CVT_FLOAT_TO_FIXEDPT(((fixedpt *)get_ptr(hid_classifier->alpha))[l]);
            }

            for( l = 0; l < node_count; l++ )
            {
                CvHidHaarTreeNode* hid_node = ((CvHidHaarTreeNode *)get_ptr(hid_classifier->node)) + l;
                CvHidHaarTreeNode* node = classifier->node + l;

                hid_node->threshold = CVT_FLOAT_TO_FIXEDPT(hid_node->threshold);
                ASSERT_DOUBLE_EQ((double)GET_FLOAT(node->threshold), fixedpt_todouble(hid_node->threshold));

                assert_sum(node->feature.rect[0].p0);
                assert_sum(node->feature.rect[0].p1);
                assert_sum(node->feature.rect[0].p2);
                assert_sum(node->feature.rect[0].p3);

                assert_sum(node->feature.rect[1].p0);
                assert_sum(node->feature.rect[1].p1);
                assert_sum(node->feature.rect[1].p2);
                assert_sum(node->feature.rect[1].p3);


                hid_node->feature.rect[0].p0 = update_sum(node->feature.rect[0].p0);
                hid_node->feature.rect[0].p1 = update_sum(node->feature.rect[0].p1);
                hid_node->feature.rect[0].p2 = update_sum(node->feature.rect[0].p2);
                hid_node->feature.rect[0].p3 = update_sum(node->feature.rect[0].p3);

                hid_node->feature.rect[0].weight =
                    CVT_FLOAT_TO_FIXEDPT(hid_node->feature.rect[0].weight);
                ASSERT_DOUBLE_EQ ((double)GET_FLOAT(node->feature.rect[0].weight),
                        fixedpt_todouble(hid_node->feature.rect[0].weight));

                hid_node->feature.rect[1].p0 = update_sum(node->feature.rect[1].p0);
                hid_node->feature.rect[1].p1 = update_sum(node->feature.rect[1].p1);
                hid_node->feature.rect[1].p2 = update_sum(node->feature.rect[1].p2);
                hid_node->feature.rect[1].p3 = update_sum(node->feature.rect[1].p3);

                hid_node->feature.rect[1].weight =
                    CVT_FLOAT_TO_FIXEDPT(hid_node->feature.rect[1].weight);
                ASSERT_DOUBLE_EQ ((double)GET_FLOAT(node->feature.rect[1].weight),
                        fixedpt_todouble(hid_node->feature.rect[1].weight));

                if (!two_rects && node->feature.rect[2].p0) {
                    assert_sum(node->feature.rect[2].p0);
                    assert_sum(node->feature.rect[2].p1);
                    assert_sum(node->feature.rect[2].p2);
                    assert_sum(node->feature.rect[2].p3);

                    hid_node->feature.rect[2].p0 = update_sum(node->feature.rect[2].p0);
                    hid_node->feature.rect[2].p1 = update_sum(node->feature.rect[2].p1);
                    hid_node->feature.rect[2].p2 = update_sum(node->feature.rect[2].p2);
                    hid_node->feature.rect[2].p3 = update_sum(node->feature.rect[2].p3);
                    hid_node->feature.rect[2].weight =
                        CVT_FLOAT_TO_FIXEDPT(hid_node->feature.rect[2].weight);
                    ASSERT_DOUBLE_EQ ((double)GET_FLOAT(node->feature.rect[2].weight),
                            fixedpt_todouble(hid_node->feature.rect[2].weight));
                }

            }

        }
    }

    // check base
    // char * tmp = (char *)cascade_fpga_base;
    // for (i = 0; i < datasize; i++) {
    //     assert(tmp[i] == 0);
    // }
    memcpy_to_shared( cascade_fpga_base, cascade_fpga_image, datasize );

    // memset(cascade_fpga_image, 0, datasize);
    free(cascade_fpga_image);
    return cascade_fpga_base;
}

#define TO_FLOAT(v) (*(float *)&(v))
int cvRunHaarClassifierCascadeSumAccOnCPU(CvHidHaarClassifierCascade* cascade, int start_stage,
        int p_offset, float variance_norm_factor)
{
    int i, j;
    for( i = start_stage; i < cascade->count; i++ ) {
        double stage_sum = 0.0;
        for( j = 0; j < cascade->stage_classifier[i].count; j++ )
        {
            CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
            CvHidHaarTreeNode* node = classifier->node;
            double t = TO_FLOAT(node->threshold) * variance_norm_factor;
            double sum = calc_sum(node->feature.rect[0],p_offset) * TO_FLOAT(node->feature.rect[0].weight);
            sum += calc_sum(node->feature.rect[1],p_offset) * TO_FLOAT(node->feature.rect[1].weight);

            if( !cascade->stage_classifier[i].two_rects && node->feature.rect[2].p0 ) {
                sum += calc_sum(node->feature.rect[2],p_offset) * TO_FLOAT(node->feature.rect[2].weight);
            }
            stage_sum += TO_FLOAT(classifier->alpha[sum >= t]);
        }
        if( stage_sum < TO_FLOAT(cascade->stage_classifier[i].threshold) )
            return -i;
    }
    return 1;
}

int cvRunHaarClassifierCascadeSumAccOnFPGA(CvHidHaarClassifierCascade* cascade_fpga, int start_stage,
        int p_offset, fixedpt variance_norm_factor)
{
    int i, j;
    fixedptd stage_sum;
    fixedptd fxpt_variance_norm_factor = (variance_norm_factor);
    for( i = start_stage; i < cascade_fpga->count; i++ ) {
        stage_sum = FIXEDPT_ZERO;
        for( j = 0; j < cascade_fpga->stage_classifier[i].count; j++ ) {
            CvHidHaarClassifier* classifier = cascade_fpga->stage_classifier[i].classifier + j;
            CvHidHaarTreeNode* node = classifier->node;

            fixedptd th = (node->threshold);
            fixedptd t = fixedptd_mul(th, fxpt_variance_norm_factor);

            fixedptd sum_1 = fixedpt_fromint(calc_sum(node->feature.rect[0], p_offset));
            fixedptd fxpt_weight_1 = (node->feature.rect[0].weight);

            sum_1 = fixedptd_mul(sum_1, fxpt_weight_1);

            fixedptd sum_2 = fixedpt_fromint(calc_sum(node->feature.rect[1], p_offset));
            fixedptd fxpt_weight_2 = (node->feature.rect[1].weight);
            sum_2 = fixedptd_mul(sum_2, fxpt_weight_2);

            fixedptd sum = fixedpt_add(sum_1, sum_2);

            if( !cascade_fpga->stage_classifier[i].two_rects && node->feature.rect[2].p0 ) {
                fixedptd sum_3 = fixedpt_fromint(calc_sum(node->feature.rect[2], p_offset));
                fixedptd fxpt_weight_3 = (node->feature.rect[2].weight);
                sum_3 = fixedptd_mul(sum_3, fxpt_weight_3);
                sum = fixedpt_add(sum, sum_3);
            }
            fixedptd alpha = (classifier->alpha[sum >= t]);
            stage_sum = fixedpt_add(stage_sum, alpha);

        }
        if( stage_sum < cascade_fpga->stage_classifier[i].threshold ) {
            // printf("exit_stage = %d\n", i);
            return -i;
        }
    }
    return 1;
}

/*
int cvRunHaarClassifierCascadeSumAccOnFPGA(CvHidHaarClassifierCascade* cascade, int start_stage, int p_offset, fixedpt variance_norm_factor)
{
    int i, j;
    fixedptd stage_sum;
#if DEBUG == 1
    double fstage_sum;
#endif
    fixedptd fxpt_variance_norm_factor = (variance_norm_factor);
    for( i = start_stage; i < cascade->count; i++ ) {
        stage_sum = FIXEDPT_ZERO;
#if DEBUG == 1
        fstage_sum = 0.0;
#endif
        for( j = 0; j < cascade->stage_classifier[i].count; j++ ) {
            CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
            CvHidHaarTreeNode* node = classifier->node;

            fixedptd th = (node->threshold);
            fixedptd t = fixedptd_mul(th, fxpt_variance_norm_factor);

#if DEBUG == 1
            double ft = fixedpt_todouble(node->threshold)*fixedpt_todouble(variance_norm_factor);

            ASSERT_DOUBLE_EQ(ft, fixedpt_todouble(t));
#endif

            fixedptd sum_1 = fixedpt_fromint(calc_sum(node->feature.rect[0], p_offset));
            ASSERT_DOUBLE_EQ((double)(calc_sum(node->feature.rect[0], p_offset)),
                    fixedpt_todouble(sum_1));
            fixedptd fxpt_weight_1 = (node->feature.rect[0].weight);

            sum_1 = fixedptd_mul(sum_1, fxpt_weight_1);

#if DEBUG == 1
            double fsum = calc_sum(node->feature.rect[0],p_offset) * fixedpt_todouble(node->feature.rect[0].weight);
            ASSERT_DOUBLE_EQ((fsum), fixedpt_todouble(sum_1));
#endif

            fixedptd sum_2 = fixedpt_fromint(calc_sum(node->feature.rect[1], p_offset));
            fixedptd fxpt_weight_2 = (node->feature.rect[1].weight);
            sum_2 = fixedptd_mul(sum_2, fxpt_weight_2);

            ASSERT_DOUBLE_EQ((double)(calc_sum(node->feature.rect[1],p_offset) * fixedpt_todouble(node->feature.rect[1].weight)),
                    fixedpt_todouble(sum_2));

            fixedptd sum = fixedpt_add(sum_1, sum_2);

#if DEBUG == 1
            fsum += calc_sum(node->feature.rect[1],p_offset) * fixedpt_todouble(node->feature.rect[1].weight);
            ASSERT_DOUBLE_EQ((fsum), fixedpt_todouble(sum));
#endif

            if( !cascade->stage_classifier[i].two_rects && node->feature.rect[2].p0 ) {
                fixedptd sum_3 = fixedpt_fromint(calc_sum(node->feature.rect[2], p_offset));
                fixedptd fxpt_weight_3 = (node->feature.rect[2].weight);
                sum_3 = fixedptd_mul(sum_3, fxpt_weight_3);
                sum = fixedpt_add(sum, sum_3);
#if DEBUG == 1
                fsum += calc_sum(node->feature.rect[2],p_offset) * fixedpt_todouble(node->feature.rect[2].weight);
#endif
            }
#if DEBUG == 1
            ASSERT_DOUBLE_EQ((fsum), fixedpt_todouble(sum));
#endif

            fixedptd alpha = (classifier->alpha[sum >= t]);
            stage_sum = fixedpt_add(stage_sum, alpha);

#if DEBUG == 1
            fstage_sum += fixedpt_todouble(classifier->alpha[sum >= t]);
            ASSERT_DOUBLE_EQ((fstage_sum), fixedpt_todouble(stage_sum));
#endif

        }
        if( stage_sum < cascade->stage_classifier[i].threshold ) {
            // printf("exit_stage = %d\n", i);
            return -i;
        }
    }
    return 1;
}
*/

typedef long long BitVector;
typedef int       AccelHandle;
#define INVAILD_HANDLE (-1)

typedef struct {
    BitVector availableAccels;
    int       totalOfAccels;
    int       numOfUsedAccels;

    pthread_cond_t  waitForAvailableAccel;
    pthread_mutex_t lock;
} Scheduler;

#define SCHEDULER_INIT(total) {\
    /* availableAccels = */ (BitVector)0, \
    /* totalOfAccels = */   total, \
    /* numOfUsedAccels = */ 0, \
    PTHREAD_COND_INITIALIZER, \
    PTHREAD_MUTEX_INITIALIZER \
}

AccelHandle getAccel(int *s);
int getAccelIfAvailable(int *s);
int freeAccel(int *s, int handle);

static Scheduler cpuSched = SCHEDULER_INIT(0);
static Scheduler fpgaSched = SCHEDULER_INIT(4);

int cvRunHaarClassifierCascadeSumAcc(CvHidHaarClassifierCascade* cascade,
        CvHidHaarClassifierCascade* cascade_fpga, int start_stage,
        int p_offset, fixedpt variance_norm_factor_fxpt, float variance_norm_factor)
{
    int i = 0;

    // printf("cvRunHaarClassifierCascadeSumAcc start\n");
    AccelHandle hCPU = INVAILD_HANDLE, hFPGA = INVAILD_HANDLE;
    if (INVAILD_HANDLE != (hCPU = getAccelIfAvailable((int *)&cpuSched))) {

        // printf("CPU AVAILABLE: ON CPU - %d.\n", hCPU);
        i = cvRunHaarClassifierCascadeSumAccOnCPU(cascade,
                start_stage, p_offset, variance_norm_factor);
        freeAccel((int *)&cpuSched, hCPU);
        // printf("FREE CPU - %d.\n", hCPU);

    } else if (INVAILD_HANDLE != (hFPGA = getAccelIfAvailable((int *)&fpgaSched))) {

        // printf("FPGA AVAILABLE: ON FPGA - %d %016lx.\n", hFPGA, fpgaSched.availableAccels);
        i = cvRunHaarClassifierCascadeSumAccOnFPGA(cascade_fpga,
                start_stage, p_offset, variance_norm_factor_fxpt);
        freeAccel((int *)&fpgaSched, hFPGA);
        // printf("FREE FPGA - %d %016lx.\n", hFPGA, fpgaSched.availableAccels);

    } else {

        // printf("BOTH UNAVAILABLE: ON CPU.\n");
        // printf("FPGA UNAVAILABLE: ON FPGA - %016lx.\n", fpgaSched.availableAccels);
        // assert(fpgaSched.availableAccels == 0x3);
        if (cpuSched.totalOfAccels > 0) {
            hCPU = getAccel((int *)&cpuSched);
            i = cvRunHaarClassifierCascadeSumAccOnCPU(cascade,
                 start_stage, p_offset, variance_norm_factor);
            freeAccel((int *)&cpuSched, hCPU);
        } else if (fpgaSched.totalOfAccels > 0) {
            // printf("Acquiring FPGA\n");
            hFPGA = getAccel((int *)&fpgaSched);
            i = cvRunHaarClassifierCascadeSumAccOnFPGA(cascade_fpga,
                    start_stage, p_offset, variance_norm_factor_fxpt);
            freeAccel((int *)&fpgaSched, hFPGA);
            // printf("FREE FPGA - %d %016lx.\n", hFPGA, fpgaSched.availableAccels);
        } else {
            return 0;
        }

    }
    // printf("exit_stage = %d\n", i);
    return i;
}

// int cvRunHaarClassifierCascadeSumAcc(CvHidHaarClassifierCascade* cascade,
//         CvHidHaarClassifierCascade* cascade_fpga, int start_stage,
//         int p_offset, fixedpt variance_norm_factor_fxpt, float variance_norm_factor)
// {
//     int i = cvRunHaarClassifierCascadeSumAccOnFPGA(cascade_fpga,
//             start_stage, p_offset, variance_norm_factor_fxpt);
//     // printf("exit_stage = %d\n", 0);
//     return i;
// }



