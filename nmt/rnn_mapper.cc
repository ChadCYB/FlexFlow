/* Copyright 2018 Stanford University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rnn_mapper.h"

RnnMapper::RnnMapper(MapperRuntime *rt, Machine machine, Processor local,
                     const char *mapper_name,
                     std::vector<Processor>* _gpus,
                     std::map<Processor, Memory>* _proc_fbmems,
                     std::vector<Processor>* _cpus)
  : DefaultMapper(rt, machine, local, mapper_name),
    gpus(*_gpus), proc_fbmems(*_proc_fbmems), cpus(*_cpus)
{}

void update_mappers(Machine machine, Runtime *runtime,
                    const std::set<Processor> &local_procs)
{
  std::vector<Processor>* gpus = new std::vector<Processor>();
  std::map<Processor, Memory>* proc_fbmems = new std::map<Processor, Memory>();
  std::vector<Processor>* cpus = new std::vector<Processor>();
  //std::map<Processor, Memory>* proc_zcmems = new std::map<Processor, Memory>();
  std::vector<Machine::ProcessorMemoryAffinity> proc_mem_affinities;
  machine.get_proc_mem_affinity(proc_mem_affinities);
  Machine::ProcessorQuery proc_query(machine);
  for (Machine::ProcessorQuery::iterator it = proc_query.begin();
      it != proc_query.end(); it++)
  {
    if (it->kind() == Processor::TOC_PROC) {
      gpus->push_back(*it);
      Machine::MemoryQuery fb_query(machine);
      fb_query.only_kind(Memory::GPU_FB_MEM);
      fb_query.best_affinity_to(*it);
      assert(fb_query.count() == 1);
      (*proc_fbmems)[*it] = *(fb_query.begin());
    }
    else if (it->kind() == Processor::LOC_PROC) {
      cpus->push_back(*it);
    }
  }

/*
  for (unsigned idx = 0; idx < proc_mem_affinities.size(); ++idx) {
    Machine::ProcessorMemoryAffinity& affinity = proc_mem_affinities[idx];
    if (affinity.p.kind() == Processor::TOC_PROC) {
      if (affinity.m.kind() == Memory::GPU_FB_MEM) {
        (*proc_fbmems)[affinity.p] = affinity.m;
      }
      else if (affinity.m.kind() == Memory::Z_COPY_MEM) {
        (*proc_zcmems)[affinity.p] = affinity.m;
      }
    }
  }

  for (std::map<Processor, Memory>::iterator it = proc_fbmems->begin();
       it != proc_fbmems->end(); it++) {
    gpus->push_back(it->first); 
  }
*/

  for (std::set<Processor>::const_iterator it = local_procs.begin();
        it != local_procs.end(); it++)
  {
    RnnMapper* mapper = new RnnMapper(runtime->get_mapper_runtime(),
                                      machine, *it, "rnn_mapper",
                                      gpus, proc_fbmems, cpus);
    runtime->replace_default_mapper(mapper, *it);
  }
}
