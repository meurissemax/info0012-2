master(base, iter, N) {
    for i = 0 to iter
        for j = 0 to base
            wait(sem_worker, 0)

        for j = 0 to base
            signal(sem_master[0])

        for j = 0 to base
            msgq_1?(j + 1, msg)

        compute where worker "j" must write back in "shm_numbers" and put it in "msg[j]"

        for j = 0 to base
            msgq_2!(j, msg[j])

        for j = 0 to base
            wait(sem_worker)

        if i == iter - 1
            shm_sorted = 1

        reset "shm_temp" to -1 everywhere

        for j = 0 to base
            signal(sem_master[1])

    for j = 0 to base
        wait(sem_worker)
}
