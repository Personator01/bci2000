////////////////////////////////////////////////////////////////////////////////
// Authors: GT
// Description: SpatialWorkingMemoryGTTask header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_SPATIALWORKINGMEMORYGTTASK_H  // makes sure this header is not included more than once
#define INCLUDED_SPATIALWORKINGMEMORYGTTASK_H


#include <map>
#include <string>
#include <sstream>
#include "ApplicationBase.h"
#include "ApplicationWindow.h"
#include "Stimulus.h"
#include "ImageStimulus.h"
#include "TextStimulus.h"
#include "Shapes.h"
#include "RandomGenerator.h"
#include <vector>
#include <random>



enum e_state {e_before_start = 2, e_background = 3, 
    e_cue = 4, e_encoding = 5, e_delay = 6, e_retrival = 7, e_feedback = 8, e_wait_to_start = 9, e_instruction = 10, e_baseline_beginning = 11,
    e_baseline_end = 12, e_score = 13};

struct Point {
    double x;
    double y;
};

Point rotatePoint90Degrees(double x, double y) {
    Point rotatedPoint;
    rotatedPoint.x = -y;
    rotatedPoint.y = x;
    return rotatedPoint;
}

class SpatialWorkingMemoryGTTask : public ApplicationBase
{
public:
    SpatialWorkingMemoryGTTask();
    ~SpatialWorkingMemoryGTTask();

    virtual void Halt();
    virtual void Preflight(const SignalProperties& Input, SignalProperties& Output) const;
    virtual void Initialize(const SignalProperties& Input, const SignalProperties& Output);
    virtual void StartRun();
    virtual void Process(const GenericSignal& Input, GenericSignal& Output);
    virtual void StopRun();

private:
    
    ApplicationWindow& mr_window;
    e_state m_current_state;

    TextStimulus* mp_text_stimulus;
    TextStimulus* rt_feedback_text_stimulus;
    GUI::Rect     m_full_rectangle;
    GUI::Rect     rect;
    RectangularShape* grid_rectangle;
    EllipticShape* encoding_dots;
    RectangularShape* distrator_rect;
    // RectangularShape* inside_distrator_rect;
    GUI::Rect     instruction_image_rectangle;
    ImageStimulus* instruction_image;
 
    // Access to the Display property.
    GUI::GraphDisplay& Display()
    {
        return mr_window;
    }
    const GUI::GraphDisplay& Display() const
    {
        return mr_window;
    }

    unsigned int  m_sample_rate;
    unsigned int  m_block_size;
    unsigned int  cross_location_x_id;
    unsigned int  cross_location_y_id;
    std::vector<bool> encoding_pattern;
    std::vector<bool> randomSequence;
    std::vector<bool> randomSequence_distrator;
    std::vector<bool> randomSequence_distrator_inside;
    std::vector<float> grid_center_coordinate_x;
    std::vector<float> grid_center_coordinate_y;
    float width_height_ratio;
    unsigned int  m_num_trials;
    unsigned int  m_trial_counter;
    unsigned int  m_runtime_counter;
    float         m_block_size_msec;
    unsigned int  m_background_duration_blocks;
    unsigned int  m_beforestart_duration_blocks;
    unsigned int  m_waitingforstart_duration_blocks;
    unsigned int  m_cue_duration_blocks;
    unsigned int  m_encoding_duration_blocks;
    unsigned int  m_delay_duration_blocks;
    unsigned int  m_retrival_duration_blocks;
    unsigned int  m_feedback_duration_blocks;
    unsigned int  m_score_duration_blocks;
    unsigned int  m_baseline_begining_duration_blocks;
    unsigned int  m_baseline_end_duration_blocks;
    unsigned int  number_of_blocks_passed;
    unsigned int  random_number;
    unsigned int  n_grid_row;
    unsigned int  n_grid_col;
    unsigned int  candidate_location_number;
    float        background_duration_msec;
    bool         grid_not_created;
    float        horizontal_rect_cross_width;
    float        horizontal_rect_cross_height;
    float        cross_coord_x;
    float        cross_coord_y;
    unsigned int cue_coordinate_row;
    unsigned int cue_coordinate_column;
    float grid_lower_y;
    float grid_upper_y;
    float grid_left_x;
    float grid_right_x;
    int          encoding_dim;
    float        grid_center_1st_row;
    float        grid_center_last_row;
    float        grid_center_1st_col;
    float             grid_center_last_col;
    unsigned int    space_from_edge;
    float           cross_location_x;
    float           cross_location_y;
    unsigned int           cross_location_row;
    unsigned int           cross_location_col;
    unsigned int    n_dots_in_retrival;
    unsigned int   bool_v_of_encoding_pattern_row1_ten_based;
    unsigned int   bool_v_of_encoding_pattern_row2_ten_based;
    unsigned int   bool_v_of_encoding_pattern_row3_ten_based;
    unsigned int   bool_v_of_encoding_pattern_row4_ten_based;
    unsigned int   bool_v_of_encoding_pattern_row5_ten_based;
    unsigned int   bool_v_of_encoding_pattern_row6_ten_based;
    std::vector<int> row_encoding_pattern_vector;
    std::vector<int> row_distrator_pattern_vector;
    std::vector<int> row_distrator_inside_pattern_vector;
    unsigned int num_stop_during_the_task;
    unsigned int active_stim_freq;

    bool response_ground_truth;
    int reaction_time;
    int retrival_evaluation_code;
    std::vector<int> reaction_time_vector_score;
    int n_correct_response;
    bool active_stim_this_trial;
    bool sham_stim_this_trial;
    bool m_is_first_trial;
    int n_distrator_outside;
    unsigned int n_distrator_inside;

    

    std::string button_to_respond_yes;
    std::string button_to_respond_no;
    std::string m_text_press_to_start;
    std::string m_text_waiting_to_start;
    std::string m_text_correct_incorrect;
    std::string m_text_reaction_time;

    std::vector<float> grid_row_y;
    std::vector<float> grid_col_x;
    std::vector<float>  grid_center_y;
    std::vector<float>  grid_center_x;
    std::vector<float> cross_candidate_location_y;
    std::vector<float> cross_candidate_location_x;
    std::vector<int> cross_candidate_location_x_ids;
    std::vector<int> cross_candidate_location_y_ids;
    std::vector<RectangularShape*> grid_rect_pointer_vector;
    std::vector<RectangularShape*> distrator_outside_pointer_vector;
    std::vector<RectangularShape*> distrator_inside_pointer_vector;
    std::vector<RectangularShape*> cross_rect_pointer_vector;
    std::vector<EllipticShape*> encoding_circle_pointer_vector;
    std::vector<bool> presented_dots_encoding;

    // Photo diode patch
    struct {
        Shape* pShape = nullptr;
        RGBColor activeColor, inactiveColor;
    } mPhotoDiodePatch;
   

    RandomGenerator m_random_generator;

    bool IsButtonPressed();
    bool IsLeftArrowButtonPressed();
    bool IsRightArrowButtonPressed();
    int RandomNumber(int min, int max);
    Point rotatePoint90Degrees(double x, double y, double centerX, double centerY);
    std::vector<bool> generateRandomSequence(int sequence_length, int n_true);
    std::vector<bool> generateRandomSequenceDistrator(std::vector<bool> base_sequence, int sequence_length, int n_true);
    int convertToBase10(const std::vector<bool>& sequence);
    bool evaluateRetrival(const std::vector<bool>& vector1, const std::vector<bool>& vector2);
    float ReactionTime();
    void Patch(bool active);
    void AllStimulusOff();


    std::vector<float> evenly_spaced_floats(float min, float max, int num) {
        std::vector<float> result(num);
        float step = (max - min) / (num - 1);
        for (int i = 0; i < num; ++i) {
            result[i] = min + i * step;
        }
        return result;
    };

    std::vector<int> evenly_spaced_ints(int min, float max, int num) {
        std::vector<int> result(num);
        int step = (max - min) / (num - 1);
        for (int i = 0; i < num; ++i) {
            result[i] = min + i * step;
        }
        return result;
    };

    template<typename T>
    std::vector<T> slice(std::vector<T> const& v, int m, int n)
    {
        auto first = v.cbegin() + m;
        auto last = v.cbegin() + n + 1;

        std::vector<T> vec(first, last);
        return vec;
    }

   // Use this space to declare any SpatialWorkingMemoryGTTask-specific methods and member variables you'll need
};

#endif // INCLUDED_SPATIALWORKINGMEMORYGTTASK_H