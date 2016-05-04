
#ifndef MITIE_TexT_CATEGORIZER_EXTRACTOR_H
#define MITIE_TexT_CATEGORIZER_EXTRACTOR_H

#include <mitie/total_word_feature_extractor.h>
#include <mitie/ner_feature_extraction.h>
#include <dlib/svm.h>
#include <dlib/vectorstream.h>
#include <dlib/hash.h>

namespace mitie
{
    class text_categorizer_extractor
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object is a simple tool for categorizing the text in
                pre-defined types.

            THREAD SAFETY
                Note that this object uses mutable internal scratch space.  Therefore, it is
                unsafe for two threads to touch the same instance of this object at a time
                without mutex locking it first.
        !*/
    public:

        text_categorizer_extractor():fingerprint(0){}
        /*!
            ensures
                - When used this object won't output any entities.   You need to either use
                  the other constructor or deserialize a saved text_categorizer_extractor to
                  get something that is useful.
        !*/

        text_categorizer_extractor(
                const std::vector<std::string>& tag_name_strings,
                const total_word_feature_extractor& fe,
                const dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<ner_sample_type>,unsigned long>& df
        );

        text_categorizer_extractor(const std::string& pureModelName,
                               const std::string& extractorName
        );

        /*!
            requires
                - df must be designed to work with fe (i.e. it must have been trained with
                  features from fe and extract_text_features()).
                - df.number_of_classes() => tag_name_strings.size()
                  (i.e. the classifier needs to predict all the possible tags and also
                  optionally a "not seen" tag which it does by predicting a value >=
                  tag_name_strings.size())
                - for all i where 0 <= i < tag_name_strings.size():
                    - df.get_labels() contains an element equal to i.
                      (All we are saying there is that the labels need to be contiguous
                      integers and that the tag_name_strings vector and the decision function
                      need to agree on the set of labels)
            ensures
                - Just loads the given objects into *this.
                - The interpretation of tag_name_strings is that it maps the output of df
                  into a meaningful type name for the text.
        !*/

        dlib::uint64 get_fingerprint(
        ) const { return fingerprint; }
        /*!
            ensures
                - returns a 64bit ID number that uniquely identifies this object instance.
                  The ID is computed based on the state of this object, so any copy of it
                  that has the same state will have the same fingerprint ID number.  This
                  is useful since down stream models that have been trained to use a
                  specific text_categorizer_extractor instance can record the fingerprint of
                  the text_categorizer_extractor they used and use that fingerprint ID to
                  verify that the text_categorizer_extractor is being used later on.
        !*/

        void predict(
                const std::vector<std::string>& sentence,
                string& text_tag,
                double& text_score
        ) const;
        /*!
            ensures
                - Runs the text categorizer on the sequence of tokenized words
                  inside sentence.  The detected tag and score are stored into
                  text_tag and text_score repectively.
                - #text_tag == the detected label for the text. Note, such tag
                  is in the range of get_tag_name_strings(), plus an optional "Unseen" label.
                - #text_score == the score for the detected label. The value
                      represents a confidence score, but does not represent a probability. Accordingly,
                      the value may range outside of the closed interval of 0 to 1. A larger value
                      represents a higher confidence. A value < 0 indicates that the label is likely
                      incorrect. That is, the canonical decision threshold is at 0.
        !*/

        void operator() (
                const std::vector<std::string>& sentence,
                string& text_tag
        ) const;
        /*!
            ensures
                - Runs the text categorizer on the sequence of tokenized words
                  inside sentence.  The detected tag and score are stored into
                  text_tag and text_score repectively.
                - #text_tag == the detected label for the text. Note, such tag
                  is in the range of get_tag_name_strings(), plus an optional "Unseen" label.
                - #text_score == the score for the detected label. The value
                      represents a confidence score, but does not represent a probability. Accordingly,
                      the value may range outside of the closed interval of 0 to 1. A larger value
                      represents a higher confidence. A value < 0 indicates that the label is likely
                      incorrect. That is, the canonical decision threshold is at 0.
        !*/

        const std::vector<std::string>& get_tag_name_strings (
        ) const { return tag_name_strings; }
        /*!
            ensures
                - Returns a vector that maps text numeric ID tags into their string labels.
        !*/

        friend void serialize(const text_categorizer_extractor& item, std::ostream& out)
        {
            int version = 2;
            dlib::serialize(version, out);
            dlib::serialize(item.fingerprint, out);
            dlib::serialize(item.tag_name_strings, out);
            serialize(item.fe, out);
            serialize(item.df, out);
        }

        friend void deserialize(text_categorizer_extractor& item, std::istream& in)
        {
            int version = 2;
            dlib::deserialize(version, in);
            dlib::deserialize(item.fingerprint, in);
            dlib::deserialize(item.tag_name_strings, in);
            deserialize(item.fe, in);
            deserialize(item.df, in);
        }

        const total_word_feature_extractor& get_total_word_feature_extractor(
        ) const { return fe; }

        const dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<ner_sample_type>,unsigned long>& get_df() const {
            return df;
        };


    private:
        void compute_fingerprint()
        {
            std::vector<char> buf;
            dlib::vectorstream sout(buf);
            sout << "fingerprint";
            dlib::serialize(tag_name_strings, sout);
            serialize(fe.get_fingerprint(), sout);
            serialize(df, sout);

            fingerprint = dlib::murmur_hash3_128bit(&buf[0], buf.size()).first;
        }

        dlib::uint64 fingerprint;
        std::vector<std::string> tag_name_strings;
        total_word_feature_extractor fe;
        dlib::multiclass_linear_decision_function<dlib::sparse_linear_kernel<ner_sample_type>,unsigned long> df;
    };
}

#endif //MITIE_TexT_CATEGORIZER_EXTRACTOR_H
