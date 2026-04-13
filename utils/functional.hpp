#pragma once

template <typename ...Fs>
struct multilambda : Fs... {
    using Fs::operator()...;
};