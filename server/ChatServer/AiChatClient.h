#pragma once
#include <string>
#include <utility>
#include <vector>
struct AiReply { bool success; std::string content; std::string error; };
class AiChatClient {
public:
    AiReply Chat(const std::string& system_prompt,
                 const std::vector<std::pair<std::string, std::string>>& messages) const;
};
