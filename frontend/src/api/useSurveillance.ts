import { useQuery } from '@tanstack/react-query';
import api from '../api';
import type { Agent } from './useAgents';
import type { LogItem } from './useLogs';

export interface SurveillanceEntry {
  agent_uuid: string;
  timestamp: string;
  name?: string;
  image_url?: string;
  data?: string;
}

async function fetchSurveillance(): Promise<SurveillanceEntry[]> {
  const { data: agentsResp } = await api.get<{ agents: Agent[] }>('/admin/agents');
  const agents = agentsResp.agents;
  const results = await Promise.all(
    agents.map(async (a) => {
      try {
        const { data } = await api.get<{ logs: LogItem[] }>(`/admin/logs/${a.uuid}`);
        return data.logs
          .filter((l) => l.type === 'Surveillance')
          .map((l) => {
            let payload: any = {};
            try { payload = JSON.parse(l.data || '{}'); } catch { payload = {}; }
            return {
              agent_uuid: a.uuid,
              timestamp: l.timestamp,
              name: payload.name,
              image_url: payload.image_url,
              data: l.data,
            } as SurveillanceEntry;
          });
      } catch {
        return [] as SurveillanceEntry[];
      }
    })
  );
  return results.flat().sort((a,b)=>new Date(b.timestamp).getTime()-new Date(a.timestamp).getTime());
}

export default function useSurveillance() {
  return useQuery({ queryKey: ['surveillance'], queryFn: fetchSurveillance });
}
