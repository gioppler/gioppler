#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <compare>
#include <iostream>
#include <ostream>
#include <random>
#include <utility>
#include <vector>

class ApproximateCounter
{
public:
    ApproximateCounter()
    :
        _mantissa{},
        _exponent{}
    {}

    void increment() {
        if (should_increment()) {
            if (_mantissa == (mantissa_values-1)) {
                if (_exponent != (exponent_values-1)) {
                    _exponent++;
                    _mantissa = 0;
                } else {
                    std::cerr << "ERROR: ApproximateCounter: overflow" << std::endl;
                }
            } else {
                _mantissa++;
            }
        }
    }

    uint32_t value() const {
        return (mantissa_values+_mantissa)*(1<<_exponent)-mantissa_values;
    }

    consteval static uint32_t maximum_value() {
        uint8_t mantissa = mantissa_values-1;
        uint8_t exponent = exponent_values-1;
        return (mantissa_values+mantissa)*(1<<exponent)-mantissa_values;
    }

    friend std::ostream& operator<<(std::ostream& os, const ApproximateCounter& ac)
    {
        os << ac.value();
        return os;
    }

private:
    static constexpr std::size_t mantissa_bits   = 4;
    static constexpr std::size_t exponent_bits   = 4;
    static constexpr uint32_t    mantissa_values = 1<<mantissa_bits;
    static constexpr uint32_t    exponent_values = 1<<exponent_bits;

    using random_uint32_engine =
        std::independent_bits_engine<std::default_random_engine, 32, uint32_t>;
    thread_local inline static random_uint32_engine random_uint32;

    uint8_t _mantissa :mantissa_bits;
    uint8_t _exponent :exponent_bits;

    bool should_increment() {
        return ((next_tag() & ((1<<_exponent)-1)) == 0);
    }

    static uint32_t next_tag() {
        return random_uint32();
    }
};

double check_range(const size_t range_max)
{
    ApproximateCounter ac1;
    double error1 = 0;

    for (int i = 1; i <= range_max; ++i) {
        ac1.increment();
        const auto value1 = ac1.value();
        error1 += std::abs(static_cast<double>(i) - value1)/value1;
    }
    return (100*error1/range_max);
}

// ------------------------------------------------------------------
void test_counter1()
{
    std::cout << "maximum value=" << ApproximateCounter::maximum_value() << std::endl;

    std::cout << "range=" << 10 << "\t   average error % = " << check_range(10) << std::endl;
    std::cout << "range=" << 100 << "\t   average error % = " << check_range(100) << std::endl;
    std::cout << "range=" << 500 << "\t   average error % = " << check_range(500) << std::endl;
    std::cout << "range=" << 1000 << "\t   average error % = " << check_range(1000) << std::endl;
    std::cout << "range=" << 5000 << "\t   average error % = " << check_range(5000) << std::endl;
    std::cout << "range=" << 10000 << "\t   average error % = " << check_range(10000) << std::endl;
    std::cout << "range=" << 50000 << "\t   average error % = " << check_range(50000) << std::endl;
    std::cout << "range=" << 100000 << "\t   average error % = " << check_range(100000) << std::endl;
    std::cout << "range=" << 500000 << "\t   average error % = " << check_range(500000) << std::endl;
    std::cout << "range=" << 1000000 << "\t   average error % = " << check_range(1000000) << std::endl;
    std::cout << "range=" << 1015792 << "\t   average error % = " << check_range(1015792) << std::endl;
}

void test_counter2()
{
    std::cout << "maximum value=" << ApproximateCounter::maximum_value() << std::endl;

    for (int runs = 1; runs < 1000; ++runs) {
        ApproximateCounter ac1;
        for (int i = 1; i <= 1000; ++i) {
            ac1.increment();
        }
        std::cout << 1000 << "\t" << ac1.value() << std::endl;
    }
}

// ------------------------------------------------------------------
class Histogram1
{
public:
    Histogram1() {
        _bins.reserve(128);
    }

    void add_event(const double duration) {
        increment_bin(duration_to_bin_index(duration));
    }

    uint64_t get_count() {
        check_count_is_updated();
        return std::lround(_count/5.0);
    }

    double get_quantile(const double quantile) const {
        check_count_is_updated();
        int64_t count_left = std::lround(_count*quantile/100);
        for (std::size_t bin = 0; bin < _bins.size(); ++bin) {
            count_left -= get_bin_avg_count(bin);
            if (count_left <= 0)   return bin_index_to_duration(bin);
        }
        return bin_index_to_duration(_bins.size()-1);
    }

    friend std::ostream& operator<<(std::ostream& os, const Histogram1& h)
    {
        os << "[";
        for (std::size_t bin = 0; bin < h._bins.size(); ++bin) {
            os << "(" << h.bin_index_to_duration(bin) << "," << h.get_bin_avg_count(bin) << ")";
        }
        os << "]";
        return os;
    }

private:
    static constexpr double log_scale  = std::log(1+1.0/8.0);
    static constexpr double nanosecond = 0.000'000'001;
    mutable uint64_t _count = 0;
    mutable bool _count_is_updated = true;
    std::vector<ApproximateCounter> _bins;

    std::size_t duration_to_bin_index(const double duration) const {
        return std::lround(std::log(static_cast<float>(duration/nanosecond))/log_scale);
    }

    double bin_index_to_duration(const std::size_t bin_index) const {
        return std::exp(static_cast<float>(bin_index)*log_scale)*nanosecond;
    }

    void increment_bin(const std::size_t bin_index) {
        _count_is_updated = false;
        if (_bins.size() < (bin_index+3)) {
            _bins.resize(bin_index+3);
        }

        if ((bin_index-2) >= 0) {
            _bins[bin_index-2].increment();
        }
        if ((bin_index-1) >= 0) {
            _bins[bin_index-1].increment();
        }
        _bins[bin_index].increment();
        _bins[bin_index+1].increment();
        _bins[bin_index+2].increment();
    }

    uint32_t get_bin_count(const std::size_t bin_index) const {
        return _bins[bin_index].value();
    }

    uint32_t get_bin_avg_count(const std::size_t bin_index) const {
        if ((bin_index-2) >= 0) {
            const double sum =
                _bins[bin_index-2].value()+
                _bins[bin_index-1].value()+
                _bins[bin_index+0].value()+
                _bins[bin_index+1].value()+
                _bins[bin_index+2].value();
            return std::lround(sum/5);
        } else if ((bin_index-1) >= 0) {
            const double sum =
                _bins[bin_index-1].value()+
                _bins[bin_index+0].value()+
                _bins[bin_index+1].value()+
                _bins[bin_index+2].value();
            return std::lround(sum/4);
        } else {
            const double sum =
                _bins[bin_index+0].value()+
                _bins[bin_index+1].value()+
                _bins[bin_index+2].value();
            return std::lround(sum/3);
        }
    }

    void check_count_is_updated() const {
        if (!_count_is_updated) {
            _count = 0;
            for (std::size_t bin = 0; bin < _bins.size(); ++bin) {
                _count += get_bin_avg_count(bin);
            }
            _count_is_updated = true;
        }
    }
};

// ------------------------------------------------------------------
// least squares moving window
// SumXY, SumX, and SumY as well as the SumX^2

class Histogram2
{
public:
    Histogram2() {
        _bins.reserve(128);
    }

    void add_event(const double duration) {
        increment_bin(duration_to_bin_index(duration));
    }

    uint64_t get_count() {
        check_count_is_updated();
        return std::lround(_count/5.0);
    }

    double get_quantile(const double quantile) const {
        check_count_is_updated();
        int64_t count_left = std::lround(_count*quantile/100);
        for (std::size_t bin = 0; bin < _bins.size(); ++bin) {
            count_left -= get_bin_avg_count(bin);
            if (count_left <= 0)   return bin_index_to_duration(bin);
        }
        return bin_index_to_duration(_bins.size()-1);
    }

    friend std::ostream& operator<<(std::ostream& os, const Histogram2& h)
    {
        os << "[";
        for (std::size_t bin = 0; bin < h._bins.size(); ++bin) {
            os << "(" << h.bin_index_to_duration(bin) << "," << h.get_bin_avg_count(bin) << ")";
        }
        os << "]";
        return os;
    }

private:
    static constexpr double log_scale  = std::log(1+1.0/8.0);
    static constexpr double nanosecond = 0.000'000'001;
    mutable uint64_t _count = 0;
    mutable bool _count_is_updated = true;
    std::vector<ApproximateCounter> _bins;

    std::size_t duration_to_bin_index(const double duration) const {
        return std::lround(std::log(static_cast<float>(duration/nanosecond))/log_scale);
    }

    double bin_index_to_duration(const std::size_t bin_index) const {
        return std::exp(static_cast<float>(bin_index)*log_scale)*nanosecond;
    }

    void increment_bin(const std::size_t bin_index) {
        _count_is_updated = false;
        if (_bins.size() < (bin_index+3)) {
            _bins.resize(bin_index+3);
        }

        if ((bin_index-2) >= 0) {
            _bins[bin_index-2].increment();
        }
        if ((bin_index-1) >= 0) {
            _bins[bin_index-1].increment();
        }
        _bins[bin_index].increment();
        _bins[bin_index+1].increment();
        _bins[bin_index+2].increment();
    }

    uint32_t get_bin_count(const std::size_t bin_index) const {
        return _bins[bin_index].value();
    }

    uint32_t get_bin_avg_count(const std::size_t bin_index) const {
        if ((bin_index-2) >= 0) {
            const double sum =
                _bins[bin_index-2].value()+
                _bins[bin_index-1].value()+
                _bins[bin_index+0].value()+
                _bins[bin_index+1].value()+
                _bins[bin_index+2].value();
            return std::lround(sum/5);
        } else if ((bin_index-1) >= 0) {
            const double sum =
                _bins[bin_index-1].value()+
                _bins[bin_index+0].value()+
                _bins[bin_index+1].value()+
                _bins[bin_index+2].value();
            return std::lround(sum/4);
        } else {
            const double sum =
                _bins[bin_index+0].value()+
                _bins[bin_index+1].value()+
                _bins[bin_index+2].value();
            return std::lround(sum/3);
        }
    }

    void check_count_is_updated() const {
        if (!_count_is_updated) {
            _count = 0;
            for (std::size_t bin = 0; bin < _bins.size(); ++bin) {
                _count += get_bin_avg_count(bin);
            }
            _count_is_updated = true;
        }
    }
};

// ------------------------------------------------------------------
void test_histogram1()
{
    Histogram h{};

    for (int event = 1; event <= 10000; ++event) {
        h.add_event(100.0+event/100.0);
    }

    // std::cout << h << std::endl;
    std::cout << "median expected=" << 150
                << "\tactual=" << h.get_quantile(50)
                << "\tcount="  << h.get_count() << std::endl;
}

// ------------------------------------------------------------------
void test_histogram2()
{
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d100{1000,25};
    std::normal_distribution<> d200{200,5};

    Histogram h{};

    for (int event = 1; event <= 10000; ++event) {
        const double value = d100(gen);
        std::cout << value << std::endl;
        h.add_event(value);
        //h.add_event(d200(gen));
    }

    // std::cout << h << std::endl;
    std::cout << "median expected=" << 1000
                << "\tactual=" << h.get_quantile(50)
                << "\tcount="  << h.get_count() << std::endl;

    std::cout << "25% quantile expected=" << (1000-25)
                << "\tactual=" << h.get_quantile(50-68.2689492137086/2)
                << "\tcount="  << h.get_count() << std::endl;

    std::cout << "75% expected=" << (1000+25)
                << "\tactual=" << h.get_quantile(50+68.2689492137086/2)
                << "\tcount="  << h.get_count() << std::endl;

}

// ------------------------------------------------------------------
int main()
{
    //test_counter1();
    test_counter2();
    //test_histogram1();
    //test_histogram2();

    return 0;
}
