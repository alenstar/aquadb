#include <stdexcept>
#include "mutex.h"

namespace util
{

Semaphore::Semaphore(uint32_t count)
{
    if (sem_init(&m_semaphore, 0, count)) {
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore() { sem_destroy(&m_semaphore); }

int Semaphore::wait()
{
    int rc = sem_wait(&m_semaphore);

    if (rc != 0) {
        // throw std::logic_error("sem_wait error");
    }
    return rc;
}

int Semaphore::notify()
{
    int rc = sem_post(&m_semaphore);
    if (rc != 0) {
        // throw std::logic_error("sem_post error");
    }
    return rc;
}

} // namespace util
