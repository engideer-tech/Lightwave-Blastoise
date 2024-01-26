#include <lightwave.hpp>

namespace lightwave {

class Bloom : public Postprocess {
public:
    explicit Bloom(const Properties& properties) : Postprocess(properties) {}

    void execute() override {
    }

    std::string toString() const override {
        return tfm::format("Bloom[]");
    }
};

} // namespace lightwave

REGISTER_POSTPROCESS(Bloom, "bloom")
