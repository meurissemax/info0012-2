worker(id, N, base) {
    search = true

    begin = (N / base) * id
    end = (N / base) * (id + 1) - 1

    divisor = 1

    if id == base - 1
        end = N - 1

    if N < base
        if id >= N
            search = false

        begin = id
        end = id

    while shm_sorted == 0
        if search
            pos = begin

            for i = begin to end
                num = shm_numbers[i]
                digit = (num / divisor) % base

                shm_temp[digit][pos] = num
                pos++

        divisor *= base;

        signal(sem_worker)
        wait(sem_master[0])

        get "nb", the number of numbers on the id_th line of "shm_temp"

        msgq_1!(id, nb)
        msgq_2?(id, write_pos)

        write back in "shm_numbers" at position "write_pos" the numbers on the id_th line of "shm_temp"

        signal(sem_worker);
        wait(sem_master[1]);

    signal(sem_worker);
}
